
current_target: 

all: map

map: map.eps

map.eps: map.coords map.links map.py
	python map.py

update:
	$(MAKE) -C mapper clean
	tar -cjf mapper.tbz2 mapper
	cp mapper.tbz2 ~/html/fritz_home/johann/
	scp mapper.tbz2 fho@qwe1:html/johann/

clean:
	rm -f *.eps *.pdf *_table.png *.tex *.out.py *.out *.pyc *.pyo core *.log stats.html map.coords map.links map.eps *.map *.html~

FORCE:

