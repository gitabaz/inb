use strict;
use warnings;
use v5.22;

sub inschar() {
    (my $char, my $pos, my $str) = @_;

}

sub slurp_file() {
    (my $file) = @_;

    my @data;

    open(my $fh, '<', $file) or die "Could not open $file!";

    while (<$fh>) {
        chomp;
        push @data, $_;
    }
    close $fh;
    say scalar @data;

    return @data;
}
