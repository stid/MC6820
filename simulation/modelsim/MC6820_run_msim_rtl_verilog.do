transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+C:/Users/stid/Desktop/MISTER/fpga/MC6820 {C:/Users/stid/Desktop/MISTER/fpga/MC6820/MC6820B.v}
vlog -vlog01compat -work work +incdir+C:/Users/stid/Desktop/MISTER/fpga/MC6820 {C:/Users/stid/Desktop/MISTER/fpga/MC6820/IRQControl.v}

vlog -vlog01compat -work work +incdir+C:/Users/stid/Desktop/MISTER/fpga/MC6820 {C:/Users/stid/Desktop/MISTER/fpga/MC6820/MC6820_bench.v}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cyclonev_ver -L cyclonev_hssi_ver -L cyclonev_pcie_hip_ver -L rtl_work -L work -voptargs="+acc"  MC6820_bench

add wave *
view structure
view signals
run -all
