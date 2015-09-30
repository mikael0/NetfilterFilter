all: test socketEnd redirect noredirect testUdp
	
redirect: server.c
	gcc -o netfilterRedirect server.c -lnfnetlink -lnetfilter_queue

socketEnd: socket_end.c
	gcc -o socketEnd socket_end.c

test: test_port.c
	gcc -o test test_port.c

testUdp: testUdp.c
	gcc -o testUdp testUdp.c

noredirect: noredirect.c
	gcc -o noredirect noredirect.c -lnfnetlink -lnetfilter_queue

clean: 
	rm -f netfilterRedirect socketEnd test noredirect nf_to_sock
