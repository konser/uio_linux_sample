PHONY:
	stap -g -t -mpagecache -p4  pagecache.stp
	rpmbuild -bb --define "_builddir `pwd`"  pagecache.spec
debug:
	stap -g -t  pagecache.stp
