QT_BUILD_PARTS += libs tools

#Qt for Windows CE c-runtime deployment
QT_CE_C_RUNTIME = no
CONFIG += minimal-config small-config medium-config large-config full-config pcre release compile_examples sse2 sse3 ssse3 sse4_1 sse4_2 avx avx2 largefile
QMAKE_QT_VERSION_OVERRIDE = 5
OBJECTS_DIR     = .obj/release_shared
MOC_DIR         = .moc/release_shared
RCC_DIR         = .rcc/release_shared
sql-plugins    += odbc psql sqlite
styles         += windows fusion windowsxp windowsvista
