
current_target: console.cgi

echo.cgi: echo.C
	g++ echo.C -lcgicc -o echo.cgi

console.cgi: console.C
	g++ console.C -ggdb -lcgicc -o console.cgi

clean:
	rm -f core core.* *.cgi

