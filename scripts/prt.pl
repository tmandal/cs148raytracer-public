#!/home/utils/perl-5.8.8/bin/perl

#
# Master script to launch single sample raytracers in parallel
#

use strict;
use warnings;

my $resolution_c = 1600;
my $resolution_r = 1200;

my $num_grids_r = 24;
my $num_grids_c = 32;

my $grid_size_r = $resolution_r / $num_grids_r;
my $grid_size_c = $resolution_c / $num_grids_c;

srand(time);
my $seed = int rand(0xffffffff);

my $rundir = "run";
mkdir $rundir;
chdir $rundir;
system("rm -rf *");

my $project = "gpu_gv100_arch_unit_ltc";
my $output_prefix = "output";
my $job_prefix = "cs148rt";
my @rtjobs = ();
my $queue = "o_cpu_8G";
my $fail = 0;

for(my $grid_r = 0; $grid_r < $num_grids_r; ++$grid_r) {
    for(my $grid_c = 0; $grid_c < $num_grids_c; ++$grid_c) {
        my $jobname = "${job_prefix}_${grid_r}_${grid_c}";
        my $output = $output_prefix . "${grid_r}_${grid_c}.png";
        push @rtjobs, $jobname;
        my $cmd = "qsub -q $queue -o run.${jobname}.log -e run.${jobname}.log2 -J $jobname -P $project  ../cs148raytracer $seed $resolution_c $resolution_r $grid_r $grid_c $grid_size_r $grid_size_c $output";
        print "$cmd\n";
        system ("$cmd");
        sleep 1;
        $fail = 1 if $?;
    }
}

sleep 20;

my $wait_for_all_jobs = join(" && ", map {"ended($_)"} @rtjobs);
my $jobname = "${job_prefix}_imgstitch";
my $cmd = "qsub -q $queue -o run.${jobname}.log -e run.${jobname}.log2 -J $jobname -P $project  -w '$wait_for_all_jobs' -Is /home/utils/Python-2.5/bin/python2.5 ../imgstitch.py $num_grids_r $num_grids_c $output_prefix";
print "\n$cmd\n";
system ("$cmd");
$fail = 1 if $?;

chdir "..";
