extractrc $(find . -name \*.rc -o -name \*.ui -o -name \*.kcfg) >> rc.cpp
xgettext --from-code=utf-8 -kde --keyword=i18n --keyword=ki18n -o po/kprinter4.pot $(find . -name \*.cpp -o -name \*.h)
rm -f rc.cpp