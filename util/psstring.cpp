/*
 * psstring.cpp
 *
 * Copyright (C) 2001-2004 Atomic Blue (http://www.atomicblue.org)
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
 *
 * A slightly improved string over csString.  Adds a couple of functions
 * that will be needed by the XML parser.
 */

#include "util/psstring.h"

bool psString::FindString(const char *border, unsigned int & pos, unsigned int & end) const
{
    pos = FindSubString(border, pos);
    if (pos == (unsigned int)-1)
        return false;

    end = pos + (int)std::strlen(border);
    end = FindSubString(border, end);
    if (end == (unsigned int)-1)
        return false;

    return true;
}

bool psString::FindNumber(unsigned int & pos, unsigned int & end) const
{
    const char *myData = this->c_str();

    while (pos < this->size() && !(isdigit(myData[pos]) ||
        myData[pos] == '-' || myData[pos] == '.'))
        pos++;

    if (pos < this->size())
    {
        end = pos;
        while (end< this->size  &&  (isdigit(myData[end]) || 
            myData[end] == '-' || myData[end] == '.'))
            end++;

        end--;
        return true;
    }
    else
        return false;
}

int psString::FindSubString(const char *sub, size_t start, bool caseInsense, bool wholeWord) const
{
    const char *myData = this->c_str();

    size_t lensub = strlen(sub);
    if ( this->size() == 0 || !lensub || lensub > this->size() )
        return -1;

    if ( caseInsense )
    {
        while ( start <= this->size() - lensub )
        {
            if (strncasecmp(sub, myData + start, lensub) != 0)
                start++;
            else
            {
                if (wholeWord)
                {
                    if ((start == 0 || !isalnum(myData[start])) &&
                        (!isalnum(myData[start+lensub])))
                        return (int)start;
                    else
                        start++;
                }
                else
                    return (int)start;
            }
        }
        return -1;
    }
    else
    {
        while (true)
        {
            const char* pWhere = strstr(myData + start, sub);
            if (pWhere)
            {
                if (!wholeWord)
                    return pWhere - myData;
                else
                {
                    start = pWhere - myData;

                    if ((start == 0 || !isalnum(myData[start])) &&
                        (!isalnum(myData[start+lensub])))
                        return (int)start;
                    else
                        start++;
                }
            }
            else
                return -1;
        }
    }
}

int psString::FindSubStringReverse(psString& sub, size_t start, bool caseInsense)
{
    const char *myData = this->c_str();

    if (this->size() == 0 || sub.size() == 0 || sub.size()>this->size())
        return -1;

    if (caseInsense)
    {
        while ( start >= 0 + sub.size() )
        {
            const char* pWhere = myData + start - sub.size();
            size_t x;
            for ( x = 0; x < sub.size(); x++ )
            {
                if ( toupper(pWhere[x]) != toupper(sub[x]) )
                    break;
            }
            if ( x < sub.size() )
                start--;
            else
                return pWhere-myData;
        }
        return -1;
    }
    else
    {
        while ( start >= 0 + sub.size() )
        {
            const char* pWhere = myData + start - sub.size();
            size_t x;
            x=0;
            for ( x = 0; x < sub.size(); x++ )
            {
                if ( pWhere[x] != sub[x] )
                    break;
            }
            if ( x < sub.size() )
                start--;
            else
                return pWhere-myData;
        }
        return -1;
    }
}

void psString::GetSubString(psString& str, size_t from, size_t to) const
{
    str.clear();

    if ( from > this->size() || from > to )
        return;

    size_t len = to - from;
    str += ( ((const char*) this) + from, len);
}

void psString::GetWord(size_t pos, psString &buff, bool wantPunct) const
{
    size_t start = pos;
    size_t end   = pos;

    if (pos > this->size())
    {
        buff="";
        return;
    }

    const char *myData = this->c_str();

    // go back to the beginning of the word
    while (start > 0 && (!isspace(myData[start])) &&
           (wantPunct || !ispunct(myData[start])) 
          )
    start--;
    if (isspace(myData[start]) || (!wantPunct && ispunct(myData[start])))
        start++;

    // search end of the word
    while (end < this->size() && (!isspace(myData[end])) &&
           (wantPunct || !ispunct(myData[end])) 
          )
    end++;

    GetSubString(buff,start,end);
}

void psString::GetWordNumber(int which, psString& buff) const
{
    buff = ::GetWordNumber((std::string) *this, which);
}
    
void psString::GetLine(size_t start, std::string& line) const
{
    size_t end  = this->find_first_of('\n',start);
    size_t end2 = this->find_first_of('\r',start);
    if (end == SIZET_NOT_FOUND)
        end = this->size();
    if (end2 == SIZET_NOT_FOUND)
        end2 = this->size();
    if (end2 < end)
        end = end2;

    line.substr(start, end - start);
}

bool psString::ReplaceSubString(const char* what, const char* with)
{
    int at;
    size_t len = strlen(what);
    if ( (at = FindSubString(what,0,false)) > -1 )
    {
        size_t where = (size_t)at;
        size_t pos = where;
        this->erase(where, len);
        this->insert(pos, with);
        return true;
    }
    else
        return false;
}

size_t psString::FindCommonLength(const psString& other) const
{
    const char *myData = this->c_str();
    const char *otherData = other.c_str();

    if (!myData || !otherData)
        return 0;

    size_t i;
    for (i=0; i<this->size(); i++)
    {
        if (myData[i] != otherData[i])
            return i;
    }
    return i;
}

//---------------------------------------------------------------------------

bool psString::IsVowel(size_t pos)
{
    switch (this->at(pos))
    {
        case 'a': case 'A':
        case 'e': case 'E':
        case 'i': case 'I':
        case 'o': case 'O':
        case 'u': case 'U':
            return true;
            
        default:
            return false;
    }
}

psString& psString::Plural()
{
    std::string subpsstring = this->substr(this->size() - 4);
    std::transform(subpsstring.begin(), subpsstring.end(), subpsstring.begin(), ::tolower);

    // Check exceptions first
    if (subpsstring == "fish")
    {
        return *this;
    }

    const char *suffix = "s";

    char token = this->at(this->size() - 1);

    switch (token) // Last char
    {
        case 's': case 'S':
        case 'x': case 'X':
        case 'z': case 'Z':
            suffix = "es";
            break;

        case 'h': case 'H':
            switch (this->at(this->size() - 2)) // Second to last char
            {
                case 'c': case 'C':
                case 's': case 'S':
                    suffix = "es";
                    break;
            }
            break;

        case 'o': case 'O':
            if (!IsVowel(this->size() - 2)) // Second to last char is consonant
                suffix = "es";
            break;

        case 'y': case 'Y':
            if (!IsVowel(this->size() - 2)) // Second to last char is consonant
            {
                this->erase(this->size() - 1);
                suffix = "ies";
            }
            break;
    }

    this->append(suffix);

    return *this;
}

void psString::Split(std::vector<std::string>& arr, char delim)
{
    //Trim
    this->erase(this->begin(), std::find_if(this->begin(), this->end(), [](int ch) {
        return !std::isspace(ch);
    }));

    if (!this->size())
        return;

    size_t pipePos = this->find_first_of(delim);
    if (pipePos == size_t(-1))
        arr.push_back((std::string) this->c_str());
    else
    {
        psString first, rest;
        first = this->substr(0, pipePos);

        //Trim
        first.erase(first.begin(), std::find_if(first.begin(), first.end(), [](int ch) {
            return !std::isspace(ch);
        }));

        if (first.size())
        {
            arr.push_back(first);

            rest = this->substr(pipePos+1, this->size()-pipePos-1);
            
            rest.Split(arr,delim);
        }
    }
}
