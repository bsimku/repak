pkgname=repak-git
pkgver=r1.a065088
pkgrel=1
pkgdesc="tool for working with RE Engine PAK files"
arch=(x86_64 i686 armv7h aarch64)
depends=(
    zlib
    zstd
)
makedepends=(
    cmake
    git
)
provides=(repak)
source=("${pkgname}::git+https://github.com/bsimku/repak")
sha256sums=(SKIP)

pkgver() {
    cd "${pkgname}"
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
    cd "${pkgname}"

    mkdir -p build
    cd build

    cmake -DCMAKE_BUILD_TYPE=None ..

    make
}
package() {
    install -Dm 755 "${pkgname}/build/src/repak" "${pkgdir}/usr/bin/repak"
}
