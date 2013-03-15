import subprocess
import time
import sys


def shell_exec(cmd):
    subprocess.check_call(cmd, shell=True)


def compile_and_run_on_board(source_files):
    TEST_FILE_PATH = '/tmp/test_prog.elf'
    CFLAGS = '-std=gnu99 -g -Wall -Wextra -I../../include -I..'
    LDFLAGS = '-std=gnu99 -g -Wall -L../../lib -lavr32b-interrupts -lm'

    print '1. Compiling test...'
    shell_exec('avr32-gcc %s %s %s -o %s' % (
               CFLAGS, ' '.join(source_files), LDFLAGS, TEST_FILE_PATH,))
    print '2. Programming...'
    shell_exec('avr32program halt')
    time.sleep(1)
    shell_exec('avr32program program -e -f0,8Mb %s' % TEST_FILE_PATH)
    print '3. Programming done. Press RESET on the board to start test.'


TESTS = {}
def register_test(name, c_files, desc):
    TESTS[name] = (c_files, desc,)


def run_test(name):
    c_files, desc = TESTS[name]
    print """\
Preparing test '%s' for execution
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  %s
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
""" % (name.upper(),
       '\n  '.join(desc.split('\n')),)

    compile_and_run_on_board(c_files)

    print
    print 'Press any key to continue...'
    raw_input()


# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


register_test('leds',
              ['leds_test.c', '../leds.c', '../utils.c'],
              """\
Generic test of the LED module.

INSTRUCTION:
Check that the LEDs operate in accordance with the commands
in leds_test.c.""")


register_test('switches',
              ['switches_test.c', '../switches.c', '../leds.c', '../utils.c'],
              """\
Generic test of the switch module.
***ASSUMES LEDS WORK OK***

INSTRUCTION:
Test various combinations of switches, and ensure that the LEDs
always reflect the active switch state.""")


register_test('dac',
              ['dac_test.c', '../dac.c'],
              """\
Generic test of the DAC module.
Uses the DAC interface, writing random data.

INSTRUCTION:
Check for a steady stream of noisy data in both output channels. (Output shoud
be identical for left and right channel.)""")


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print 'Usage: %s [test name|all]' % sys.argv[0]
        sys.exit(-1)

    to_run = list(TESTS.keys()) if sys.argv[1] == 'all' else [sys.argv[1]]
    to_run.sort()
    for test_name in to_run:
        run_test(test_name)
