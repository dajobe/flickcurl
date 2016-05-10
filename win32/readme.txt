Only static library is built. I assume that dependencies are also static libraries.
Paths for includes and necessary macros definitions are in Macros.props

Dependencies:
curl build is almost hassle-free on windows.
libiconv and libxml are unsupported mess, I'll add patches against specific versions later.
