
/**************************************************************************
*                                                                         *
*   (C) Copyright 2008                                                    *
*   Texas Instruments, Inc.  <www.ti.com>                                 * 
*                                                                         *
*   See file CREDITS for list of people who contributed to this project.  *
*                                                                         *
**************************************************************************/


/**************************************************************************
*                                                                         *
*   This file is part of the DaVinci Flash and Boot Utilities.            *
*                                                                         *
*   This program is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful, but   *
*   WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
*   General Public License for more details.                              *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  if not, write to the Free Software          *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307   *
*   USA                                                                   *
*                                                                         *
**************************************************************************/

/* --------------------------------------------------------------------------
    FILE        : GenAIS.cs
    PROJECT     : TI Booting and Flashing Utilities
    AUTHOR      : Daniel Allred
    DESC        : Command-line app to generate an AIS file (comparable to genAIS.pl Perl script) 
 ----------------------------------------------------------------------------- */

using System;
using System.Text;
using System.IO;
using AISGenLib;
using System.Reflection;
using UtilLib.IO;

[assembly: AssemblyTitle("GenAIS")]
[assembly: AssemblyVersion("1.02.*")]

namespace TIBootAndFlash
{
    partial class Program
    {
        private struct ProgramCmdParams
        {
            public String inputfileName;

            public String outFileName;

            public Boolean valid;
        }
        
        /// <summary>
        /// Function to display help in case command-line is invalid
        /// </summary>
        static void DispHelp()
        {
            Console.Write("Usage:\n\n");
            Console.Write("GenAIS [Options] <Input File Name>\n");
            Console.Write("\t" + "<Option> can be any of the following:\n");
            Console.Write("\t\t" + "-h                   \tShow this help screen.\n");
            Console.Write("\t\t" + "-o <Output File Name>\tExplicitly specify the output filename.\n");
            Console.Write("\t\t" + "                     \tDefault is input file name with .ais extension.\n");
            Console.Write("\n");
            Console.Write("Note that this program is only guaranteed to work for producing binary NOR images.\n");
            Console.Write("Use genAIS perl scripts to get other formats.\n");
        }

        /// <summary>
        /// Function to parse the command line
        /// </summary>
        /// <param name="args">Array of command-line arguments</param>
        /// <returns>Struct of the filled in program arguments</returns>
        static ProgramCmdParams ParseCmdLine(String[] args)
        {
            ProgramCmdParams myCmdParams = new ProgramCmdParams();
            Boolean[] argsHandled = new Boolean[args.Length];

            UInt32 numUnhandledArgs, numHandledArgs = 0;
            String s;

            // Check for no argumnents
            if (args.Length == 0)
            {
                myCmdParams.valid = false;
                return myCmdParams;
            }

            // Set Defaults
            myCmdParams.valid = true;
            myCmdParams.outFileName = null;
            myCmdParams.inputfileName = null;

            // Initialize array of handled argument booleans to false
            for (int i = 0; i < argsHandled.Length; i++)
                argsHandled[i] = false;

            // For loop to check for all dash options
            for (int i = 0; i < args.Length; i++)
            {
                s = args[i];
                if (s.StartsWith("-"))
                {
                    switch (s.Substring(1).ToLower())
                    {
                        case "o":
                            myCmdParams.outFileName = args[i + 1];
                            argsHandled[i + 1] = true;
                            numHandledArgs++;
                            break;
                        default:
                            myCmdParams.valid = false;
                            break;
                    }
                    argsHandled[i] = true;
                    numHandledArgs++;
                }
            }
            numUnhandledArgs = (UInt32)(args.Length - numHandledArgs);
            
            // Check to make sure we are still valid
            if ( (!myCmdParams.valid) || (numUnhandledArgs != 1) || (argsHandled[args.Length-1]))
            {
                myCmdParams.valid = false;
                return myCmdParams;
            }
           
            // Get input file
            FileInfo fi = new FileInfo(args[args.Length-1]);
            if (fi.Exists)
            {
                //myCmdParams.inputfileName = fi.Name;
                myCmdParams.inputfileName = args[args.Length - 1];
                String extension = Path.GetExtension(fi.Name);
                if (myCmdParams.outFileName == null)
                {
                    myCmdParams.outFileName = Path.GetFileNameWithoutExtension(fi.Name) + ".ais";
                }
                if (!extension.Equals(".out",StringComparison.OrdinalIgnoreCase))
                {
                    Console.WriteLine("Warning: Input filename does not have .out extension!");
                    Console.WriteLine("\tIt may not be a valid COFF file.");
                }
            }
            else
            {
                Console.WriteLine("File not found.");
                myCmdParams.valid = false;
                return myCmdParams;
            }

            return myCmdParams;
        }
        
        /// <summary>
        /// Main program.
        /// </summary>
        /// <param name="args">Input commandline arguments</param>
        /// <returns>Return code</returns>
        static Int32 Main(String[] args)
        {           
            // Assumes that in AssemblyInfo.cs, the version is specified as 1.0.* or the like,
            // with only 2 numbers specified;  the next two are generated from the date.
            System.Version v = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
            
            // v.Build is days since Jan. 1, 2000, v.Revision*2 is seconds since local midnight
            Int32 buildYear = new DateTime( v.Build * TimeSpan.TicksPerDay + v.Revision * TimeSpan.TicksPerSecond * 2 ).AddYears(1999).Year;
            
            // Begin main code
            Console.Clear();
            Console.WriteLine("-----------------------------------------------------");
            Console.WriteLine("   TI NOR AIS File Generator for " + devString   );
            Console.WriteLine("   (C) "+buildYear+", Texas Instruments, Inc."        );
            Console.WriteLine("   Ver. "+v.Major+"."+v.Minor.ToString("D2")          );
            Console.WriteLine("-----------------------------------------------------");
            Console.Write("\n\n");            

            
            // Parse the input command line parameters
            ProgramCmdParams cmdParams = ParseCmdLine(args);
            if (!cmdParams.valid)
            {
                DispHelp();
                return -1;
            }   
            
            // Now proceed with main program
            FileStream tempAIS_fs = null;
            Byte[] AISData;
                                    
            // Create a new C642x AIS generator object
            AISGen_C642x generator = new AISGen_C642x();
            generator.CRCType = CRCCheckType.SECTION_CRC;

            // Do the AIS generation
            try
            {
                AISData = AISGen.GenAIS(cmdParams.inputfileName, "emifa", generator);
			    tempAIS_fs = new FileStream(cmdParams.outFileName, FileMode.Create, FileAccess.Write);
			    tempAIS_fs.Write(AISData, 0, (int)AISData.Length);
			    tempAIS_fs.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.StackTrace);
                Console.WriteLine("Unhandled Exception!!! Application will now exit.");
                return -1;
            }
            
            Console.WriteLine("AIS generation is complete.");
            return 0;
        }
    }
}
