pkgname=libcregex 
pkgver=2.0
source=("${pkgname}_${pkgver}-0.tar.gz") 
pkgrel=1
arch=(any)
sha256sums=('SKIP')

package() {
	cmake --install .. --prefix "$pkgdir/usr"
}
