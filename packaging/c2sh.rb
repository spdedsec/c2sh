class C2sh < Formula
  desc "Production-minded C17 interactive Unix shell"
  homepage "https://github.com/spdedsec/c2sh"
  url "https://github.com/spdedsec/c2sh/archive/refs/tags/v0.1.0.tar.gz"
  sha256 "74b7ba27e77316bc503f68738d311e9c81e5b383d26c60674cdcfa77f298cbe3"
  license "GPL-3.0-or-later"

  def install
    system "make", "CC=#{ENV.cc}", "CFLAGS=#{ENV.cflags} -D_POSIX_C_SOURCE=200809L"
    bin.install "build/c2sh"
    man1.install "docs/c2sh.1"
  end

  test do
    assert_equal "HELLO", shell_output("#{bin}/c2sh -c 'printf hello | tr a-z A-Z'").strip
  end
end
