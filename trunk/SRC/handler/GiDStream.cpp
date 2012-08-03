/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.13 $
// $Date: 2009-10-13 21:17:42 $
// $Source: /usr/local/cvs/OpenSees/SRC/handler/GiDStream.cpp,v $

#include <GiDStream.h>
#include <Vector.h>
#include <Matrix.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <ID.h>
#include <Channel.h>
#include <Message.h>

using std::cerr;
using std::ios;
using std::setiosflags;
using std::string;
using std::ifstream;
using std::getline;

#define OPS_STREAM_TAGS_GiDStream 101

GiDStream::GiDStream(int indent)
  :OPS_Stream(OPS_STREAM_TAGS_GiDStream), 
   fileOpen(0), fileName(0), indentSize(indent), numIndent(-1),
   attributeMode(false), numTag(0), sizeTags(0), tags(0), sendSelfCount(0), theChannels(0), numDataRows(0),
   mapping(0), maxCount(0), sizeColumns(0), theColumns(0), theData(0), theRemoteData(0), 
   xmlOrderProcessed(0), xmlString(0), xmlStringLength(0), numXMLTags(0), xmlColumns(0)
{
  if (indentSize < 1) indentSize = 1;
  indentString = new char[indentSize+1];
  for (int i=0; i<indentSize; i++)
    strcpy(&indentString[i]," ");
}

GiDStream::GiDStream(const char *name, openMode mode, int indent)
  :OPS_Stream(OPS_STREAM_TAGS_GiDStream), 
   fileOpen(0), fileName(0), indentSize(indent), numIndent(-1),
   attributeMode(false), numTag(0), sizeTags(0), tags(0), sendSelfCount(0), theChannels(0), numDataRows(0),
   mapping(0), maxCount(0), sizeColumns(0), theColumns(0), theData(0), theRemoteData(0), 
   xmlOrderProcessed(0), xmlString(0), xmlStringLength(0), numXMLTags(0), xmlColumns(0)
{
  if (indentSize < 1) indentSize = 1;
  indentString = new char[indentSize+1];

  for (int i=0; i<indentSize; i++)
    strcpy(&indentString[i]," ");

  this->setFile(name, mode);
}

GiDStream::~GiDStream()
{
  if (fileOpen == 1) {
    this->close();
  }

  /*
  if (theChannels != 0) {
    static ID lastMsg(1);
    if (sendSelfCount > 0) {
      for (int i=0; i<sendSelfCount; i++) 
	theChannels[i]->sendID(0, 0, lastMsg);
    } else
	theChannels[0]->recvID(0, 0, lastMsg);
    delete [] theChannels;
  }
  */

  if (indentString != 0)
    delete [] indentString;

  if (fileName != 0)
    delete [] fileName;

  if (sendSelfCount > 0) {

    for (int i=0; i<=sendSelfCount; i++) {
      if (theColumns != 0)
	if (theColumns[i] != 0)
	  delete theColumns[i];

      if (theData != 0)
	if (theData[i] != 0)
	  delete [] theData[i];
      
      if (theRemoteData != 0)
	if (theRemoteData[i] != 0)
	  delete theRemoteData[i];
    }
    if (theData != 0) delete [] theData;
    if (theRemoteData != 0) delete [] theRemoteData;
    if (theColumns != 0) delete [] theColumns;
    if (sizeColumns != 0) delete sizeColumns;
  }    

  if (sendSelfCount < 0) {

    if (theColumns[0] != 0)
      delete theColumns[0];

    delete [] theColumns;
  }    

  if (xmlColumns != 0)
    delete xmlColumns;
}

int 
GiDStream::setFile(const char *name, openMode mode)
{
  if (name == 0) {
    std::cerr << "GiDStream::setFile() - no name passed\n";
    return -1;
  }

  // first create a copy of the file name
  if (fileName != 0) {
    if (strcmp(fileName, name) != 0)
      delete [] fileName;
    fileName = 0;
  }

  if (fileName == 0) {
    fileName = new char[strlen(name)+5];
    if (fileName == 0) {
      std::cerr << "GiDStream::setFile() - out of memory copying name: " << name << std::endl;
      return -1;
    }
    
    // copy the strings
    strcpy(fileName, name);
  }

  // if file already open, close it
  if (fileOpen == 1) {
    theFile.close();
    fileOpen = 0;
  }

  if (mode == 0)
    theOpenMode = OVERWRITE;
  else
    theOpenMode = APPEND;

  return 0;
}

int 
GiDStream::open(void)
{
  // check setFile has been called
  if (fileName == 0) {
    std::cerr << "GiDStream::open(void) - no file name has been set\n";
    return -1;
  }

  // if file already open, return
  if (fileOpen == 1) {
    return 0;
  }

  if (sendSelfCount > 0) {
    strcat(fileName,".0");
  }
  
  // open file
  if (theOpenMode == OVERWRITE) 
    theFile.open(fileName, ios::out);
  else
    theFile.open(fileName, ios::out| ios::app);

  theOpenMode = APPEND;

  if (theFile.bad()) {
    std::cerr << "WARNING - GiDStream::open()";
    std::cerr << " - could not open file " << fileName << std::endl;
    fileOpen = 0;
    return -1;
  } else
    fileOpen = 1;


  if (sendSelfCount >= 0) {

	//numIndent++;

  } 
  theFile << setiosflags(ios::fixed);

  return 0;
}

int 
GiDStream::close(void)
{
  if (fileOpen == 1) {
    for (int i=0; i<numTag; i++) {
      this->endTag();
    }

    //theFile << "</OpenSees>\n";
    theFile.close();
  }

  fileOpen = 0;
  return 0;
}


int 
GiDStream::setPrecision(int prec)
{
  if (fileOpen == 0)
    this->open();

  if (fileOpen != 0)
    theFile << std::setprecision(prec);

  return 0;
}

int 
GiDStream::setFloatField(floatField field)
{
  if (fileOpen == 0)
    this->open();

  if (field == FIXEDD) {
    if (fileOpen != 0)
      theFile << setiosflags(ios::fixed);
  }
  else if (field == SCIENTIFIC) {
    if (fileOpen != 0)
      theFile << setiosflags(ios::scientific);
  }

  return 0;
}


int 
GiDStream::tag(const char *tagName)
{
  if (fileOpen == 0)
    this->open();

  //
  // copy tagName to end of list of tags
  //

  // if tags array not large enough, expand
  //if (numTag == sizeTags) {
  //  int nextSize = 2*sizeTags;
  //  
  //  if (nextSize == 0) nextSize = 32;
  //  char **nextTags = new char *[nextSize];
  //  if (nextTags != 0) {
  //    for (int i=0; i<sizeTags; i++)
	//nextTags[i] = tags[i];
  //    for (int j=sizeTags+1; j<nextSize; j++)
	//nextTags[j] = 0;
  //    sizeTags = nextSize;
  //  } else {
  //    sizeTags = 0;
  //    delete [] tags;
  //    tags = 0;
  //    return -1;
  //  }
  //  
  //  if (tags != 0)
	//delete [] tags;
  //  
  //  tags = nextTags;
  //} 


  // copy string and assign to end of array
  //char *newTag = new char[strlen(tagName) +1];
  //strcpy(newTag, tagName);



  //if (sendSelfCount != 0 && numTag != 0) {
  //  if (attributeMode == true) 
  //    (*xmlColumns)(numXMLTags) += 2;
  //  else
  //    (*xmlColumns)(numXMLTags) += 1;
  //}
  
  //tags[numTag++] = newTag;

  //  if (sendSelfCount == 0 || (strstr(tagName,"Data") != 0)) {
  //  if (attributeMode == true) {
  //    theFile << " ";
  //  }
    
    // output the xml for it to the file
    
    //numIndent++;
    //this->indent();
    //theFile << " " << tagName;
    
    //attributeMode = true;

    /*  } else {
    numIndent++;
    int nextXmlStringLength = xmlStringLength + numIndent*indentSize + strlen(tagName) + 5;
    char *nextXmlString = new char [nextXmlStringLength];

    if (xmlString != 0) {
      strncpy(nextXmlString, xmlString, xmlStringLength);
      delete [] xmlString;
    } else
      nextXmlString[0]='\0';

    xmlString = nextXmlString;
    xmlStringLength = nextXmlStringLength;
    
    if (attributeMode == true) {
      strcat(xmlString,">\n");
    } else {
      strcat(xmlString,"\n");
    }

    for (int i=0; i<numIndent; i++)
      strcat(xmlString, indentString);      
    strcat(xmlString, "<");      
    strcat(xmlString, tagName);      
    
    attributeMode = true;
  }
    */
  return 0;
}


int 
GiDStream::tag(const char *tagName, const char *value)
{
  if (fileOpen == 0)
    this->open();

  //  if (sendSelfCount == 0) {
  //if (attributeMode == true) {
  //  theFile << " \n";
  //}
    
    // output the xml for it to the file
    //numIndent++;
    //this->indent();
  //  theFile << "  " << tagName << " " << value << " " << tagName << " " << endln;
    //numIndent--;

  //if (sendSelfCount != 0 && numTag != 0)
  //  (*xmlColumns)(numXMLTags) += 1;
    
  //  attributeMode = false;
    /*  } else {

    numIndent++;
    int nextXmlStringLength = xmlStringLength + numIndent*indentSize + 2*strlen(tagName) + strlen(value) + 6;
    char *nextXmlString = new char [nextXmlStringLength];

    if (xmlString != 0) {
      strncpy(nextXmlString, xmlString, xmlStringLength);
      delete [] xmlString;
    } else
      nextXmlString[0]='\0';

    xmlString = nextXmlString;
    xmlStringLength = nextXmlStringLength;
    
    if (attributeMode == true) {
      strcat(xmlString,">\n");
    } 

    for (int i=0; i<numIndent; i++)
      strcat(xmlString, indentString);      

    numIndent--;

    strcat(xmlString, "<");      
    strcat(xmlString, tagName);      
    strcat(xmlString, ">");      
    strcat(xmlString, value);      
    strcat(xmlString, "</");      
    strcat(xmlString, tagName);      
    strcat(xmlString, ">\n");          

    attributeMode = false;
  }
    */
  return 0;
}

int 
GiDStream::endTag()
{
  //  if (sendSelfCount == 0) {
    if (numTag != 0) {
    //  if (attributeMode == true) {
	//theFile << " \n";
	//delete [] tags[numTag-1];
	//numTag--;
    //  } else {
	////this->indent();
	////theFile << "End "<< tags[numTag-1]; //
	//delete [] tags[numTag-1];
	//numTag--;
    //  }    
    //  
    //  attributeMode = false;
    //  //numIndent--;
	//
    //  if (sendSelfCount != 0)
	//(*xmlColumns)[numXMLTags] += 1;
	//
    //  if (numIndent == -1) 
	//numXMLTags++;

      return 0;
    }
    else {
  
    /*if (numTag != 0) {
      if (attributeMode == true) {
	int nextXmlStringLength = xmlStringLength +  4;
	char *nextXmlString = new char [nextXmlStringLength];
	nextXmlString[0]='\0';
	if (xmlString != 0) {
	  strncpy(nextXmlString, xmlString, xmlStringLength);
	  delete [] xmlString;
	}
      
	xmlString = nextXmlString;
	xmlStringLength = nextXmlStringLength;

	strcat(xmlString,"/>\n");
	delete [] tags[numTag-1];
	numTag--;

      } else {

	int nextXmlStringLength = xmlStringLength + numIndent*indentSize + strlen(tags[numTag-1]) + 5;
	char *nextXmlString = new char [nextXmlStringLength];


	if (xmlString != 0) {
	  strncpy(nextXmlString, xmlString, xmlStringLength);
	  delete [] xmlString;
	} else
	  nextXmlString[0]='\0';
										  
	xmlString = nextXmlString;
	xmlStringLength = nextXmlStringLength;

	for (int i=0; i<numIndent; i++)
	  strcat(xmlString, indentString);      
	strcat(xmlString, "</");      
	strcat(xmlString, tags[numTag-1]);      
	strcat(xmlString, ">\n");      

	delete [] tags[numTag-1];
	numTag--;
      }
	  
      attributeMode = false;
      numIndent--;

      if (numIndent == -1) {

	  delete [] xmlString;
	  xmlString = 0;
	  xmlStringLength = 0;
	} */

      return 0;
    }
   
    return -1;
}

int 
GiDStream::attr(const char *name, int value)

{  if (fileOpen == 0)
    this->open();
  
  //  if (sendSelfCount == 0 ) {

  //    theFile << " " << name << "=\"" << value << "\"";

      /*  } else {

    static char intRep[30];
    sprintf(intRep, "%d", value);

    int nextXmlStringLength = xmlStringLength + strlen(name) + strlen(intRep) + 5;
    char *nextXmlString = new char [nextXmlStringLength];

    if (xmlString != 0) {
      strncpy(nextXmlString, xmlString, xmlStringLength);
      delete [] xmlString;
    } else
      nextXmlString[0]='\0';

    xmlString = nextXmlString;
    xmlStringLength = nextXmlStringLength;

    strcat(xmlString, " ");      
    strcat(xmlString, name);   
    strcat(xmlString, "=\"");         
    strcat(xmlString, intRep);         
    strcat(xmlString, "\"");         

  }
      */  
  return 0;
}

int 
GiDStream::attr(const char *name, double value)
{
  if (fileOpen == 0)
    this->open();

  //  if (sendSelfCount == 0) {

  //  theFile << " " << name << "=\"" << value << "\"";

    /*  } else {

    char intRep[30];
    sprintf(intRep, "%e", value);

    int nextXmlStringLength = xmlStringLength + strlen(name) + strlen(intRep) + 5;
    char *nextXmlString = new char [nextXmlStringLength];

    if (xmlString != 0) {
      strncpy(nextXmlString, xmlString, xmlStringLength);
      delete [] xmlString;
    } else
      nextXmlString[0]='\0';

    xmlString = nextXmlString;
    xmlStringLength = nextXmlStringLength;

    strcat(xmlString, " ");      
    strcat(xmlString, name);   
    strcat(xmlString, "=\"");         
    strcat(xmlString, intRep);         
    strcat(xmlString, "\"");         
  }
    */
  return 0;
}

int 
GiDStream::attr(const char *name, const char *value)
{
  if (fileOpen == 0)
    this->open();

  //  if (sendSelfCount == 0) {

    //theFile << " " << name << "=\"" << value << "\"";

    /*  } else {

    int nextXmlStringLength = xmlStringLength + strlen(name) + strlen(value) + 5;
    char *nextXmlString = new char [nextXmlStringLength];

    if (xmlString != 0) {
      strncpy(nextXmlString, xmlString, xmlStringLength);
      delete [] xmlString;
    } else
      nextXmlString[0]='\0';

    xmlString = nextXmlString;
    xmlStringLength = nextXmlStringLength;

    strcat(xmlString, " ");      
    strcat(xmlString, name);   
    strcat(xmlString, "=\"");         
    strcat(xmlString, value);         
    strcat(xmlString, "\"");         
  }
    */
  return 0;
}

int 
GiDStream::write(Vector &data)
{
  if (fileOpen == 0)
    this->open();

  //
  // if not parallel, just write the data
  //

  if (sendSelfCount == 0) {
    numDataRows++;
    
    if (attributeMode == true) {
      theFile << " \n";
      attributeMode = false;
  //    numIndent++;
    }
    
  //  this->indent();
    (*this) << data;  
    return 0;
  }

  //
  // otherwise parallel
  //


  //
  // process xml info in the files
  //


  //
  // send data if not p0

  if (sendSelfCount < 0) {
    if (data.Size() != 0) {
      theChannels[0]->sendVector(0, 0, data);
      return 0;
    } else {
      return 0;
    }
  }

  //
  // if p0 recv the data & write it out sorted
  //

  // recv data

  numDataRows++;
  
  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  //  numIndent++;
  }
  
  //this->indent();
  
  for (int i=0; i<=sendSelfCount; i++) {
    int numColumns = (*sizeColumns)(i);

    double *dataI = theData[i];
    if (i == 0) {
      for (int j=0; j<numColumns; j++) {
	dataI[j] = data(j);
      }
    } else { 
      if (numColumns != 0) {
	theChannels[i-1]->recvVector(0, 0, *(theRemoteData[i]));
      }
    }
  }

  Matrix &printMapping = *mapping;

  // write data
  for (int i=0; i<maxCount+1; i++) {
    int fileID = (int)printMapping(0,i);
    int startLoc = (int)printMapping(1,i);
    int numData = (int)printMapping(2,i);
    double *data = theData[fileID];
    for (int j=0; j<numData; j++)
      theFile << data[startLoc++] << " ";
  }
  theFile << "\n";
  theFile.flush();
  return 0;
}

int 
GiDStream::write(int data)
{
  if (fileOpen == 0)
    this->open();

  theFile << data << "\t";
  theFile.flush();
  return 0;
}


OPS_Stream& 
GiDStream::write(const char *s,int n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << s ;

  return *this;
}

OPS_Stream& 
GiDStream::write(const unsigned char*s,int n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << s;

  return *this;
}
OPS_Stream& 
GiDStream::write(const signed char*s,int n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << s;

  return *this;
}

OPS_Stream& 
GiDStream::write(const void *s, int n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << s;

  return *this;
}

OPS_Stream& 
GiDStream::write(const double *s, int n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0) {
    for (int i=0; i<n; i++)
      theFile << s[i] << " ";
    theFile << endln;
  }

  return *this;
}


OPS_Stream& 
GiDStream::operator<<(char c)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << c;

  return *this;
}
OPS_Stream& 
GiDStream::operator<<(unsigned char c)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << c;

  return *this;
}
OPS_Stream& 
GiDStream::operator<<(signed char c)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << c;

  return *this;
}
OPS_Stream& 
GiDStream::operator<<(const char *s)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  // note that we do the flush so that a "/n" before
  // a crash will cause a flush() - similar to what 
  if (fileOpen != 0) {
    theFile << s;
    theFile.flush();
  }

  return *this;
}
OPS_Stream& 
GiDStream::operator<<(const unsigned char *s)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << s;

  return *this;
}
OPS_Stream& 
GiDStream::operator<<(const signed char *s)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << s;

  return *this;
}
OPS_Stream& 
GiDStream::operator<<(const void *p)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

/*
  if (fileOpen != 0)
    theFile << p;
*/
  return *this;
}
OPS_Stream& 
GiDStream::operator<<(int n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << 1.0*n;

  return *this;
}
OPS_Stream& 
GiDStream::operator<<(unsigned int n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << 1.0*n;

  return *this;
}
OPS_Stream& 
GiDStream::operator<<(long n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

/*
  if (fileOpen != 0)
    theFile << n;
*/
  return *this;
}
OPS_Stream& 
GiDStream::operator<<(unsigned long n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

/*
  if (fileOpen != 0)
    theFile << n;
*/
  return *this;
}
OPS_Stream& 
GiDStream::operator<<(short n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

/*
  if (fileOpen != 0)
    theFile << n;
*/
  return *this;
}
OPS_Stream& 
GiDStream::operator<<(unsigned short n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

/*
  if (fileOpen != 0)
    theFile << n;
*/
  return *this;
}
OPS_Stream& 
GiDStream::operator<<(bool b)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

/*
  if (fileOpen != 0)
    theFile << b;
*/
  return *this;
}
OPS_Stream& 
GiDStream::operator<<(double n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << n;

  return *this;
}
OPS_Stream& 
GiDStream::operator<<(float n)
{
  if (fileOpen == 0)
    this->open();

  if (attributeMode == true) {
    theFile << " \n";
    attributeMode = false;
  }

  if (fileOpen != 0)
    theFile << n;

  return *this;
}


int 
GiDStream::sendSelf(int commitTag, Channel &theChannel)
{
  sendSelfCount++;

  Channel **theNextChannels = new Channel *[sendSelfCount];
  for (int i=0; i<sendSelfCount-1; i++)
    theNextChannels[i] = theChannels[i];
  theNextChannels[sendSelfCount-1] = &theChannel;
  if (theChannels != 0)
    delete [] theChannels;
  theChannels = theNextChannels;

  static ID idData(3);
  int fileNameLength = 0;
  if (fileName != 0)
    fileNameLength = strlen(fileName);

  idData(0) = fileNameLength;

  if (theOpenMode == OVERWRITE)
    idData(1) = 0;
  else
    idData(1) = 1;

  idData(2) = sendSelfCount;

  if (theChannel.sendID(0, commitTag, idData) < 0) {
    opserr << "GiDStream::sendSelf() - failed to send id data\n";
    return -1;
  }

  if (fileNameLength != 0) {
    Message theMessage(fileName, fileNameLength);
    if (theChannel.sendMsg(0, commitTag, theMessage) < 0) {
      opserr << "GiDStream::sendSelf() - failed to send message\n";
      return -1;
    }
  }
  
  return 0;
}

int 
GiDStream::recvSelf(int commitTag, Channel &theChannel, FEM_ObjectBroker &theBroker)
{
  static ID idData(3);

  sendSelfCount = -1;
  theChannels = new Channel *[1];
  theChannels[0] = &theChannel;

  if (theChannel.recvID(0, commitTag, idData) < 0) {
    opserr << "GiDStream::recvSelf() - failed to recv id data\n";
    return -1;
  }

  int fileNameLength = idData(0);
  if (idData(1) == 0)
    theOpenMode = OVERWRITE;
  else
    theOpenMode = APPEND;

  if (fileNameLength != 0) {
    if (fileName != 0)
      delete [] fileName;
    fileName = new char[fileNameLength+5];
    if (fileName == 0) {
      opserr << "GiDStream::recvSelf() - out of memory\n";
      return -1;
    }

    Message theMessage(fileName, fileNameLength);
    if (theChannel.recvMsg(0, commitTag, theMessage) < 0) {
      opserr << "GiDStream::recvSelf() - failed to recv message\n";
      return -1;
    }

    int tag = idData(2);

    sprintf(&fileName[fileNameLength],".%d",tag);

    if (this->setFile(fileName, theOpenMode) < 0) {
      opserr << "GiDStream::GiDStream() - setFile() failed\n";
      if (fileName != 0) {
	delete [] fileName;
	fileName = 0;
      }
    }
  }
  
  return 0;
}

void
GiDStream::indent(void)
{
  if (fileOpen != 0)
    for (int i=0; i<numIndent; i++)
      theFile << indentString;
}

int
GiDStream::setOrder(const ID &orderData)
{
  if (fileOpen == 0)
    this->open();

  if (sendSelfCount == 0)
    return 0;

  if (xmlOrderProcessed == 0) {
    xmlColumns = new ID(orderData.Size());
    xmlOrderProcessed = 1;
  } else if (xmlOrderProcessed == 1) {
    //this->mergeXML();
    xmlOrderProcessed = 2;
  } 

  if (sendSelfCount < 0) {
    static ID numColumnID(1);
    int numColumn = orderData.Size();
    numColumnID(0) = numColumn;
    theChannels[0]->sendID(0, 0, numColumnID);

    theColumns = new ID *[1];
    theColumns[0] = new ID(orderData);
      
    if (numColumn != 0)
      theChannels[0]->sendID(0, 0, orderData);
  }

  if (sendSelfCount > 0) {      

    sizeColumns = new ID(sendSelfCount+1);
    theColumns = new ID *[sendSelfCount+1];
    theData = new double *[sendSelfCount+1];
    theRemoteData = new Vector *[sendSelfCount+1];
    
    int numColumns = orderData.Size();
    (*sizeColumns)(0) = numColumns;
    if (numColumns != 0) {
      theColumns[0] = new ID(orderData);
      theData[0] = new double [numColumns];
    } else {
      theColumns[0] = 0;
      theData[0] = 0;
    }      
    theRemoteData[0] = 0;

    maxCount = 0;
    if (numColumns != 0)
      maxCount = orderData(numColumns-1);

    // now receive orderData from the other channels
    for (int i=0; i<sendSelfCount; i++) { 
      static ID numColumnID(1);	  
      if (theChannels[i]->recvID(0, 0, numColumnID) < 0) {
	opserr << "GiDStream::setOrder - failed to recv column size for process: " << i+1 << endln;
	return -1;
      }

      numColumns = numColumnID(0);

      (*sizeColumns)(i+1) = numColumns;
      if (numColumns != 0) {
	theColumns[i+1] = new ID(numColumns);
	if (theChannels[i]->recvID(0, 0, *theColumns[i+1]) < 0) {
	  opserr << "GiDStream::setOrder - failed to recv column data for process: " << i+1 << endln;
	  return -1;
	}

	if (numColumns != 0 && (*theColumns[i+1])[numColumns-1] > maxCount)
	  maxCount = (*theColumns[i+1])[numColumns-1];
	
	theData[i+1] = new double [numColumns];
	theRemoteData[i+1] = new Vector(theData[i+1], numColumns);
      } else {
	theColumns[i+1] = 0;
	theData[i+1] = 0;
	theRemoteData[i+1] = 0;
      }
    }

    ID currentLoc(sendSelfCount+1);
    ID currentCount(sendSelfCount+1);
	
    if (mapping != 0)
      delete mapping;

    mapping = new Matrix(3, maxCount+1);

    Matrix &printMapping = *mapping;
	
    for (int i=0; i<=sendSelfCount; i++) {
      currentLoc(i) = 0;
      if (theColumns[i] != 0)
	currentCount(i) = (*theColumns[i])[0];
      else
	currentCount(i) = -1;
    }

    int count =0;
    while (count <= maxCount) {
      for (int i=0; i<=sendSelfCount; i++) {
	if (currentCount(i) == count) {
	  printMapping(0,count) = i;
	  
	  int maxLoc = theColumns[i]->Size();
	  int loc = currentLoc(i);
	  int columnCounter = 0;
	  
	  printMapping(1,count) = loc;
	  
	  while (loc < maxLoc && (*theColumns[i])(loc) == count) {
	    loc++;
	    columnCounter++;
	  }
	  
	  printMapping(2,count) = columnCounter;
	  
	  currentLoc(i) = loc;
	  
	  if (loc < maxLoc)
	    currentCount(i) = (*theColumns[i])(loc);		
	  else
	    currentCount(i) = -1; 		
	}
      }
      count++;
    }
  }

  if (theChannels != 0) {
    static ID lastMsg(1);
    if (sendSelfCount > 0) {
      for (int i=0; i<sendSelfCount; i++) 
	theChannels[i]->sendID(0, 0, lastMsg);
    } else
	theChannels[0]->recvID(0, 0, lastMsg);
  }

  return 0;
}
