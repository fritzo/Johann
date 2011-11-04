
set terminal postscript eps

set xrange [0 to 1]

set output "Z_vs_eps.eps"
set title "Z vs. epsilon"
plot "funs_of_eps.text" using ($1):($2) smooth unique
set output

set output "P_vs_eps.eps"
set title "P vs. epsilon"
plot "funs_of_eps.text" using ($1):($3) smooth unique
set output

set output "N_vs_eps.eps"
set title "N vs. epsilon"
plot "funs_of_eps.text" using ($1):($4) smooth unique
set output

set output "H_vs_eps.eps"
set title "H vs. epsilon"
plot "funs_of_eps.text" using ($1):($5) smooth unique
set output

set output "iH_vs_eps.eps"
set title "iH vs. epsilon"
plot "funs_of_eps.text" using ($1):($6) smooth unique
set output

set output "R_vs_eps.eps"
set title "R vs. epsilon"
plot "funs_of_eps.text" using ($1):($7) smooth unique
set output




