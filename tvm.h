/*
 * tvm.h
 *
 * See the README file for copyright information
 *
 */

#include "epgd.h"
#include <math.h>

#define TVM_VERSION  "0.1.4"

//***************************************************************************
// Tvm2
//***************************************************************************

class Tvm2 : public Plugin
{
   public:

      Tvm2();
      virtual ~Tvm2();

      int init(cEpgd* aObject, int aUtf8);
      int initDb();
      int exitDb();
      int atConfigItem(const char* Name, const char* Value);

      const char* getSource()   { return "tvm"; }

      int getPicture(const char* imagename, const char* fileRef, MemoryStruct* data);
      int processDay(int day, int fullupdate, Statistic* stat);
      int cleanupAfter();
      const char* userAgent()   { return "Mozilla/4.0 (compatible; Clickfinder 5.3; Windows NT 5.2)"; }

      int ready();

   protected:

      int processFile(const char* extid, MemoryStruct* data, const char* fileName, const char* fileRef);
      int downloadImageFile(const char* extid);
      int createXml(const char* extid, MemoryStruct* xmlData);
      int zipValid(MemoryStruct* data);

      xsltStylesheetPtr pxsltStylesheet;
      cDbStatement* stmtMarkOldEvents;
      cDbStatement* stmtSetUpdFlgByFileref;
      cDbStatement* selectDistBySource;
      cDbStatement* selectId;
      cDbValue* valueFileRef;
      
      // config

      int timeout;
};

// TVM specific stuff
/*
    Time is measured by TV Movie in TVM-ticks, which are linear with some unknown time being zero. I have
    aligned manually two points on this time axis, which are epoch (in TVM-ticks) and unixTimeEpoch (in
    UNIX time, seconds since 1970). The conversion factor is given by factor.
    Gives the TV-Movie-ticks equivalent to the UNIX time ggiven in unixTimeEpoch. 
*/

const uint64_t epoch = 4675708794760826425LL;
const double factor = 95443717 / 60.0;            // Number of TVM-ticks in per second. 
const unsigned int unixTimeEpoch = 1233270600;

//***************************************************************************
// Field
//***************************************************************************

class Field 
{
   public:

      enum ContentType 
      { 
         ctInt      = 3, 
         ctDateTime = 7, 
         ctString   = 8,
         ctTwoByte  = 11 
      };

      Field(FILE* file)        { read(file); }

      const string& getString() const  { return stringContent; }
      uint64_t getNumeric()     const  { return numeric; }
      bool isNumeric()          const  { return type == ctInt || type == ctTwoByte; }
      bool isString()           const  { return type == ctString || type == ctDateTime; }

      string getXmlString()
      {
         string xml = "";
         
         for (unsigned int i = 0; i < stringContent.size(); i++) 
         {
            char c = stringContent[i];

            if (c == '<') 
            {
               if (stringContent.substr(i, 4) == "<br>") 
               {
                  xml += "\n\n";
                  i += 3;
               } 
               else 
                  xml += "&lt;";
            } 

            else if (c == '>')
               xml += "&gt;";
            else if (c == '&')
               xml += "&amp;";
            else if (c == '"')
               xml += "&quot;";
            else
               xml += c;
         }

         return xml;
      }

   private:

      void read(FILE* file) 
      {
         type = (ContentType)readInt(file, 2);

         switch (type) 
         {
            case ctString:  readString(file);            break;
            case ctInt:     numeric = readInt(file, 4);  break;

            case ctDateTime: 
            {
               char buffer[30];

               numeric = readInt(file, 8);
               unixTime = unixTimeEpoch + (int)round((numeric-epoch)/factor) - 1;
               
               strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", gmtime(&unixTime));
               stringContent = buffer;

               break;
            }

            case ctTwoByte:
            {
               numeric = readInt(file, 2);

               if (numeric == 0xFFFF)
                  numeric = 1;

               break;
            }
         }
      }

      void readString(FILE* file)
      {
         int length = readInt(file, 2);
         
         for (int i = 0; i < length; i++) 
         {
            int ch = fgetc(file);
            stringContent.push_back(ch);
         }
      }
      
      static uint64_t readInt(FILE* file, int bytes) 
      {
         uint64_t numeric = 0;

         for (int i = 0; i < bytes; i++) 
         {
            int byte = fgetc(file);
            numeric += (uint64_t)byte << (8*i);
         }

         return numeric;
      }
      
      ContentType type;
      string stringContent;
      uint64_t numeric;
      time_t unixTime;
};
