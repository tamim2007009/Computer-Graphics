

####################################################
## Basys3 Rev B - 4-bit Parallel Adder XDC
## Top module: parallel_adder_4bit
####################################################

## Switches: Input A (SW0-SW3)
set_property -dict { PACKAGE_PIN V17 IOSTANDARD LVCMOS33 } [get_ports {a[0]}]
set_property -dict { PACKAGE_PIN V16 IOSTANDARD LVCMOS33 } [get_ports {a[1]}]
set_property -dict { PACKAGE_PIN W16 IOSTANDARD LVCMOS33 } [get_ports {a[2]}]
set_property -dict { PACKAGE_PIN W17 IOSTANDARD LVCMOS33 } [get_ports {a[3]}]

## Switches: Input B (SW4-SW7)
set_property -dict { PACKAGE_PIN W15 IOSTANDARD LVCMOS33 } [get_ports {b[0]}]
set_property -dict { PACKAGE_PIN V15 IOSTANDARD LVCMOS33 } [get_ports {b[1]}]
set_property -dict { PACKAGE_PIN W14 IOSTANDARD LVCMOS33 } [get_ports {b[2]}]
set_property -dict { PACKAGE_PIN W13 IOSTANDARD LVCMOS33 } [get_ports {b[3]}]

## Switch: Carry Input (SW8)
set_property -dict { PACKAGE_PIN V2 IOSTANDARD LVCMOS33 } [get_ports cin]

## LEDs: Sum Output (LED0-LED3)
set_property -dict { PACKAGE_PIN U16 IOSTANDARD LVCMOS33 } [get_ports {sum[0]}]
set_property -dict { PACKAGE_PIN E19 IOSTANDARD LVCMOS33 } [get_ports {sum[1]}]
set_property -dict { PACKAGE_PIN U19 IOSTANDARD LVCMOS33 } [get_ports {sum[2]}]
set_property -dict { PACKAGE_PIN V19 IOSTANDARD LVCMOS33 } [get_ports {sum[3]}]

## LED: Carry Output (LED4)
set_property -dict { PACKAGE_PIN W18 IOSTANDARD LVCMOS33 } [get_ports cout]

####################################################
## Configuration (recommended)
####################################################
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]






