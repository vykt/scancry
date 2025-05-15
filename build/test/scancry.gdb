# print sc_opt
define popt
	if $argc != 1
		printf "Use: popt <sc_opt>\n"
	else
		p *((sc::opt *) $arg0)
	end
end

# hex print sc_opt
define pxopt
	if $argc != 1
		printf "Use: pxopt <sc_opt>\n"
	else
		p/x *((sc::opt *) $arg0)
	end
end


# print sc_opt_ptrscan
define pptr
	if $argc != 1
		printf "Use: pptr <sc_opt_ptrscan>\n"
	else
		p *((sc::opt_ptrscan *) $arg0)
	end
end

# hex print sc_opt_ptrscan
define pxptr
	if $argc != 1
		printf "Use: pxptr <sc_opt_ptrscan>\n"
	else
		p/x *((sc::opt_ptrscan *) $arg0)
	end
end
