'''
/*HEADER**********************************************************************
 *
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * Freescale Confidential and Proprietary - use of this software is
 * governed by the Freescale MQX RTOS License distributed with this
 * material. See the MQX_RTOS_LICENSE file distributed for more
 * details.
 *
 *****************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************
 *
 * Comments:
 *
 *END************************************************************************/
'''
from distutils.core import setup
import py2exe, sys, os
sys.argv.append('py2exe')


setup(
	options = {'py2exe': { 
		'bundle_files'	: 1,
		'includes'		: ['win32api'],
		'dll_excludes'	: [ "mswsock.dll", "powrprof.dll" ]
	}},
	console = [{'script': "image.py"}],
	zipfile = None,
)



setup(
	options = {'py2exe': { 
		'bundle_files'	: 1,
		'includes'		: ['win32api'],
		'dll_excludes'	: [ "mswsock.dll", "powrprof.dll" ]
	}},
	console = [{'script': "ddcopy.py"}],
	zipfile = None,
)


