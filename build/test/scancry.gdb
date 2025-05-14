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
		printf "Use: popt <sc_opt>\n"
	else
		p/x *((sc::opt *) $arg0)
	end
end
