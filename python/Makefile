
current_target:
	python SKR.py

all:

init.appTable: ExprDB.py Ob.py App.py AppTable.py axioms.py
	python makeInitDB.py

default.appTable: init.appTable
	python makeDefaultDB.py



axioms.tex: makeAxiomsTex.py axioms.py
	python makeAxiomsTex.py

exprs.tex: makeExprsTex.py exprs.py
	python makeExprsTex.py

ExprDB.dvi: ExprDB.tex axioms.tex exprs.tex
	latex ExprDB.tex


clean:
	rm -f *.pyc *.pyo *.aux *.dvi *.log *.appTable *.eps tags

