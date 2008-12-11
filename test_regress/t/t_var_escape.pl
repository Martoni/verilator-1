#!/usr/bin/perl
if (!$::Driver) { use FindBin; exec("$FindBin::Bin/bootstrap.pl", @ARGV, $0); die; }
# DESCRIPTION: Verilator: Verilog Test driver/expect definition
#
# Copyright 2003-2008 by Wilson Snyder. This program is free software; you can
# redistribute it and/or modify it under the terms of either the GNU
# General Public License or the Perl Artistic License.

compile (
	 # Access is so we can dump waves
	 v_flags2 => [$Self->{v3}?'-trace':' +access+rwc'],
	 );

execute (
	 check_finished=>1,
	 );

if ($Self->{v3}) {
    file_grep     ("$Self->{obj_dir}/simx.vcd", qr/\$enddefinitions/x);
    my $sig = quotemeta("bra[ket]slash/dash-colon:9");
    file_grep     ("$Self->{obj_dir}/simx.vcd", qr/$sig/);
}

ok(1);
1;