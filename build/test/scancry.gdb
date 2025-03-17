# source memcry functions
source memcry.gdb


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

# print areas of map area set returned by the C interface
define pcmaset
	if $argc != 1
		printf "Use: pmapa <*sc_map_area_set>\n"
	else
		# print header
		printf " --- [MAP AREA SET] ---\n"
		
		# bootstrap iteration
		set $iter = 0

		# for every area
		while $iter != $arg0->len

			# fetch & typecast next area
			set $area = ((mc_vm_area) *((((cm_lst_node *) $iter->data) + $iter)->data))

			# fetch relevant entries for this area
			set $id         = $area->id
			set $basename   = $area->basename
			set $start_addr = $area->start_addr
			set $end_addr   = $area->end_addr

			# print relevant entries of this area
			printf "%-3d: 0x%-12lx - 0x%-12lx | \"%s\"\n", $id, $start_addr, $end_addr, $basename

			# advance iteration
			set $iter = $iter + 1
		end
	end 
end


# session dependent (modify from here onwards)
tb main
run
layout src
