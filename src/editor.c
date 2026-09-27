#include "editor.h"
#include "util.h"
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <ctype.h>
extern void history_add(Shell*,const char*);
static struct termios orig;static int raw_on=0;
static void disable_raw(void){if(raw_on){tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig);raw_on=0;}}
static int enable_raw(void){if(!isatty(STDIN_FILENO))return -1;if(tcgetattr(STDIN_FILENO,&orig)<0)return -1;struct termios r=orig;r.c_lflag&=(tcflag_t)~(ECHO|ICANON|IEXTEN|ISIG);r.c_iflag&=(tcflag_t)~(IXON|ICRNL);r.c_oflag|=OPOST;if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&r)<0)return -1;raw_on=1;atexit(disable_raw);return 0;}
static void redraw(const char*p,const char*b,size_t len,size_t cur){printf("\r\033[2K%s%s",p,b);printf("\r\033[%zuC",strlen(p)+cur);fflush(stdout);(void)len;}
static char*history_find(Shell*sh,const char*q){for(size_t i=sh->history_len;i>0;i--){if(strstr(sh->history[i-1].text,q))return sh->history[i-1].text;}return NULL;}
static void complete(Shell*sh,char*b,size_t*len,size_t*cur,const char*p){(void)sh;(void)cur;size_t st=*len;while(st>0&&!isspace((unsigned char)b[st-1]))st--;char prefix[PATH_MAX];size_t n=*len-st;if(n>=sizeof(prefix))return;memcpy(prefix,b+st,n);prefix[n]='\0';char dir[PATH_MAX],base[PATH_MAX];char*slash=strrchr(prefix,'/');if(slash){size_t dn=(size_t)(slash-prefix);if(dn>=sizeof(dir))return;memcpy(dir,prefix,dn);dir[dn]='\0';if(!dir[0])strcpy(dir,"/");strncpy(base,slash+1,sizeof(base)-1);base[sizeof(base)-1]='\0';}else{strcpy(dir,".");strncpy(base,prefix,sizeof(base)-1);base[sizeof(base)-1]='\0';}
 DIR*d=opendir(dir);if(!d)return;char first[PATH_MAX]="";int count=0;struct dirent*e;while((e=readdir(d))){if(strncmp(e->d_name,base,strlen(base))==0){if(count==0)snprintf(first,sizeof(first),"%s",e->d_name);count++;}}closedir(d);if(count==1){char replacement[PATH_MAX];if(slash)snprintf(replacement,sizeof(replacement),"%.*s%s",(int)(slash-prefix+1),prefix,first);else snprintf(replacement,sizeof(replacement),"%s",first);size_t rl=strlen(replacement);if(st+rl+1<4096){memcpy(b+st,replacement,rl);*len=st+rl;b[*len]='\0';*cur=*len;redraw(p,b,*len,*cur);}}
}
char*editor_readline(Shell*sh,const char*prompt){if(enable_raw()<0){fputs(prompt,stdout);fflush(stdout);char*line=NULL;size_t n=0;if(getline(&line,&n,stdin)<0){free(line);return NULL;}size_t l=strlen(line);while(l&&(line[l-1]=='\n'||line[l-1]=='\r'))line[--l]='\0';return line;}
 char b[4096]="";size_t len=0,cur=0;size_t hist=sh->history_len;printf("%s",prompt);fflush(stdout);
 for(;;){unsigned char c;if(read(STDIN_FILENO,&c,1)!=1){disable_raw();return NULL;}if(c=='\r'||c=='\n'){putchar('\n');disable_raw();b[len]='\0';return strdup(b);}if(c==4){if(len==0){putchar('\n');disable_raw();return NULL;}continue;}if(c==3){printf("^C\n");disable_raw();return strdup("");}
  if(c==127||c==8){if(cur){memmove(b+cur-1,b+cur,len-cur+1);cur--;len--;redraw(prompt,b,len,cur);}continue;}
  if(c==9){complete(sh,b,&len,&cur,prompt);continue;}
  if(c==18){char q[4096];memcpy(q,b,len);q[len]='\0';char*h=history_find(sh,q);if(h){strncpy(b,h,sizeof(b)-1);b[sizeof(b)-1]='\0';len=cur=strlen(b);redraw(prompt,b,len,cur);}continue;}
  if(c==27){char seq[2];if(read(STDIN_FILENO,seq,1)!=1)continue;if(seq[0]=='['){if(read(STDIN_FILENO,seq+1,1)!=1)continue;switch(seq[1]){case 'A':if(hist>0){hist--;strncpy(b,sh->history[hist].text,sizeof(b)-1);b[sizeof(b)-1]='\0';len=cur=strlen(b);redraw(prompt,b,len,cur);}break;case 'B':if(hist<sh->history_len){hist++;if(hist==sh->history_len){b[0]='\0';len=cur=0;}else{strncpy(b,sh->history[hist].text,sizeof(b)-1);b[sizeof(b)-1]='\0';len=cur=strlen(b);}redraw(prompt,b,len,cur);}break;case 'C':if(cur<len)cur++;redraw(prompt,b,len,cur);break;case 'D':if(cur)cur--;redraw(prompt,b,len,cur);break;case 'H':cur=0;redraw(prompt,b,len,cur);break;case 'F':cur=len;redraw(prompt,b,len,cur);break;case '3':{char t;if(read(STDIN_FILENO,&t,1)==1&&t=='~'&&cur<len){memmove(b+cur,b+cur+1,len-cur);len--;redraw(prompt,b,len,cur);}break;}default:break;}}continue;}
  if(c>=32&&c<127&&len<sizeof(b)-1){memmove(b+cur+1,b+cur,len-cur+1);b[cur++]=(char)c;len++;redraw(prompt,b,len,cur);}
 }
}
