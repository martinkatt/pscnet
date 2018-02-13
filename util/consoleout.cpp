/*
* serverconsole.cpp
*
* Copyright (C) 2001 Atomic Blue (info@planeshift.it, http://www.atomicblue.org) 
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation (version 2 of the License)
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* Author: Matthias Braun <MatzeBraun@gmx.de>
*/

#include "consoleout.h"

FILE* errorLog = NULL;
static FILE* outputfile = NULL;
static ConsoleOutMsgClass maxoutput_stdout = CON_SPAM;
static ConsoleOutMsgClass maxoutput_file = CON_SPAM;

int ConsoleOut::shift = 0;
std::string *ConsoleOut::strBuffer = NULL;
bool ConsoleOut::atStartOfLine = true;
bool ConsoleOut::promptDisplayed = false;

void ConsoleOut::SetMaximumOutputClassStdout (ConsoleOutMsgClass con)
{
    // always output command output on stdout
    if(con < CON_CMDOUTPUT)
    {
        maxoutput_stdout = CON_CMDOUTPUT;
    }
    else
    {
        maxoutput_stdout = con;
    }
}

void ConsoleOut::SetMaximumOutputClassFile (ConsoleOutMsgClass con)
{
    maxoutput_file = con;
}

ConsoleOutMsgClass ConsoleOut::GetMaximumOutputClassStdout()
{
    return maxoutput_stdout;
}

ConsoleOutMsgClass ConsoleOut::GetMaximumOutputClassFile()
{
    return maxoutput_file;
}

void ConsoleOut::SetOutputFile (const char* filename, bool append)
{
    if (outputfile)
    {
        fclose (outputfile);
        outputfile = NULL;
    }
    if (filename)
    {
        outputfile = fopen (filename, append ? "a" : "w");
    }
}

void ConsoleOut::Intern_Printf (ConsoleOutMsgClass con, const char* string, ...)
{
    va_list args;
    va_start(args, string);
    Intern_VPrintf(con,string,args);
    va_end(args);
}

void ConsoleOut::Intern_VPrintf (ConsoleOutMsgClass con, const char* string, va_list args)
{
    std::string output;
    std::stringstream time_buffer; //holds the time string to be appended to each line
    bool ending_newline = false;
    //output.FormatV(string, args); //formats the output
	
	//get time stamp
    time_t curtime = time(NULL);
    struct tm *loctime;
    loctime = localtime (&curtime);
    time_buffer << asctime(loctime); //formats the time string to be appended
    time_buffer.str(time_buffer.str().erase(time_buffer.str().size() - 1));
    time_buffer << ", ";
    // Append any shift
    for (int i=0; i < shift; i++)
    {
        time_buffer << "  ";
    }
    output.insert(0, time_buffer.str()); //add it to the starting of the string

    // Format output with timestamp at each line and apply shifts
    
    if(output.at(output.size() - 1) == '\n')  //check if there is an ending new line to avoid substitution there
    {
        output.erase(output.size() - 1);
        ending_newline = true;
    }

    time_buffer.str(time_buffer.str().insert(0, "\n")); 
    //add the leading new line in the time string
    size_t start_pos;
    std::string replace = "\n";

    while((start_pos = output.find(replace, start_pos)) != std::string::npos) {
        output.replace(start_pos, replace.length(), time_buffer.str());
        start_pos += time_buffer.str().length(); // ...
    }
    //adds the string to be appended to the output string
    
    if(ending_newline) //restore the ending newline if it was removed
        output.append("\n");

    // Now that we have output correctly formated check where to send the output

    // Check for stdout
    if (con <= maxoutput_stdout)
    {
        if (strBuffer)
        {
            strBuffer->append(output);
        }
        else
        {
            if (promptDisplayed)
            {
                printf("\n");
                promptDisplayed = false;
            }
            printf("%s", output.c_str());
            fflush(stdout);
        }
    }
    

    // Check for output file
    if (outputfile && con <= maxoutput_file)
    {
        fprintf(outputfile, "%s", output.c_str());
    }
    // Check for error log
    if (con == CON_ERROR ||
        con == CON_BUG)
    {
        if(!errorLog)
        {
            errorLog = fopen("errorlog.txt","w");
        }

        if(errorLog)
        {
            fprintf(errorLog, "%s", output.c_str());
            fflush(errorLog);
        }
    }


#ifdef USE_READLINE
    rl_redisplay();
#endif
}

void ConsoleOut::Intern_Printf_LogOnly(ConsoleOutMsgClass con,
                                       const char *string, ...)
{
    va_list args;
    va_start(args, string);
    Intern_VPrintf_LogOnly(con,string,args);
    va_end(args);
}

void ConsoleOut::Intern_VPrintf_LogOnly(ConsoleOutMsgClass con,
                                       const char *string, va_list args)
{
    if (outputfile && con <= maxoutput_file)
    {
        for (int i=0; i < shift; i++)
        {
            vfprintf (outputfile, "  ", args);
        }
        vfprintf (outputfile, string, args);
        fflush (outputfile);
    }
}

void ConsoleOut::Shift()
{
    shift ++;
}

void ConsoleOut::Unshift()
{
    shift --;
}

void ConsoleOut::SetPrompt(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::string buffer;
    //buffer.FormatV(format, args);
    va_end(args);

    if (!atStartOfLine)
    {
        printf("\n");
        atStartOfLine = true;
    }

    auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    printf("%8u) %s", msecs, buffer.c_str());
    promptDisplayed = true;
}

