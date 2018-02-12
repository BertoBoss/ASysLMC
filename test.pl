:-use_module(library(aleph)).


read_all(X) :- current_output(O),
	       open('log.txt',write,S),
	       set_output(S),
	       aleph:read_all(X),
	       set_output(O),
	       close(S).

induce_mute :- current_output(O),
	       open('log.txt',write,S),
	       set_output(S),
	       aleph:induce,
	       set_output(O),
	       close(S).

bottom(X) :- aleph:bottom(X).

show(X) :- aleph:show(X).

rdhyp(X) :- current_output(O),
	    current_input(I),
	    open('log.txt',write,S2),
	    open(X,read,S),
	    set_input(S),
	    set_output(S2),
	    aleph:rdhyp,
	    set_input(I),
	    set_output(O),
	    close(S2),
	    close(S). 

covers(X) :- current_output(O),
	     open(X,write,S),
	     set_output(S),
	     aleph:covers,
	     flush,
	     set_output(O),
	     close(S).

coversn(X) :- current_output(O),
	      open(X,write,S),
	      set_output(S),
	      aleph:coversn,
	      flush,
	      set_output(O),
	      close(S).

sat(X,Y) :- current_output(O),
	    open(Y,write,S),
	    set_output(S),
	    aleph:sat(X),
	    flush,
	    set_output(O),
	    close(S).
