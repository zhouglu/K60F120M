#/*HEADER**********************************************************************
# *
# * Copyright 2010 Freescale Semiconductor, Inc.
# *
# * Freescale Confidential and Proprietary - use of this software is
# * governed by the Freescale MQX RTOS License distributed with this
# * material. See the MQX_RTOS_LICENSE file distributed for more
# * details.
# *
# *****************************************************************************
# *
# * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
# * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# * THE POSSIBILITY OF SUCH DAMAGE.
# *
# *****************************************************************************
# *
# * Comments:
# * This tool processes the prepare.txt file and creates a batch program which 
# * generates the final codesize reports as specifed
# *
# * usage:  prepare.pl > do_all.bat
# *
# *END************************************************************************/

$DEBUG = 0;
$INFILE = shift;
$INFILE = "prepare.txt" if not $INFILE; 

open MAPFILES, $INFILE or die "Can not open \"$INFILE\" input file";

$inrep = 0;
$nl = $DEBUG ? "\n" : "";

while($line = <MAPFILES>)
{
  chomp $line;
  
  # comment line
  next if $line =~ /^\s*\#/;
  
  # report begins here
  if($line =~ /^\s*report\s+\"(.*)\"/)
  {
    print "..\\codesize.pl -o $1 ";
    #print " -K";  # kernel symbols (not working with uv4)
    print " -W";   # debugging warning messages
    print "$nl";
    $inrep = 1;
    next;
  }
  
  # need to be in report to accept next input lines
  next if not $inrep;
  
  # map file entry, parse board, tool etc.
  if($line =~ /app\\(\w+)\.(\w+)\\(\w+)_(\w+)\\/)
  {
    # new line -> process collected mapfiles
    $board = $1;
    $mapfmt = $2;
    $kernel = $3;
    $cscfg = $4;
    
    print " -c $mapfmt -n \"$board\" -n2 \"$kernel kernel\" -n3 \"$cscfg app\" $line $nl";
  }
  else
  {
    # this report is done 
    print "\n\n";
    $inrep = 0;
  }
}

print "\n";

close MAPFILES;
0;
