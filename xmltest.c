
#include <map>
#include <string>

#include <math.h>
#include <stdint.h>
#include <lib/config.h>
#include <lib/common.h>

const char* logPrefix = "";
int createXml(const char* extid, MemoryStruct* xmlData);

//***************************************************************************
// Field
//***************************************************************************

const uint64_t epoch = 4675708794760826425LL;
const double factor = 95443717 / 60.0;            // Number of TVM-ticks in per second. 
const unsigned int unixTimeEpoch = 1233270600;

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

      const std::string& getString() const  { return stringContent; }
      uint64_t getNumeric()     const  { return numeric; }
      bool isNumeric()          const  { return type == ctInt || type == ctTwoByte; }
      bool isString()           const  { return type == ctString || type == ctDateTime; }

      std::string getXmlString()
      {
         std::string xml = "";
         
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
               
               //strftime(buffer, 30, "%Y-%m-%d %k:%M:%S (%Z)", gmtime(&unixTime));
               unixTime += tmeSecondsPerHour;               
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
      std::string stringContent;
      uint64_t numeric;
      time_t unixTime;
};

//***************************************************************************
// Main
//***************************************************************************

int main(int argc, char** argv)
{
   MemoryStruct xmlData;
   char* command;
   char password[7+TB];
   char* lang;
   char* filename = 0;
   cEpgConfig::logstdout = yes;
   cEpgConfig::loglevel = 0;
   const char* extid;

   setlocale(LC_CTYPE, "");
   setlocale(LC_TIME, "");

   if (argc < 3)
   {
      tell(0, "Usage: extst <path> <tvmid>");
      return 1;
   }

   extid = argv[2];
   asprintf(&filename, "%s/tvdaten-premium-%s.cftv.zip", argv[1], extid);
   memset(password, 0xb7, 7);
   password[7] = 0;

   asprintf(&command, "unzip -o -P %s %s", password, filename);

   if (system(command) < 0 || !fileExists(filename))
   {
      printf("Extracting of '%s' failed\n", filename);
      free(command);
      return 1;
   }

   printf("Extracting of '%s' done\n", filename);
   free(command);
   free(filename);

   createXml(extid, &xmlData);

   return 0;
}

//***************************************************************************
// Create Xml
//***************************************************************************

int createXml(const char* extid, MemoryStruct* xmlData)
{
   char* filename;
   std::map<int, std::map<std::string, std::string> > table;
   std::map<int, std::map<std::string, std::string> >::iterator iter;
   
   printf("Creating xml of %s\n", extid);  

   // -----------------------------------------------
   // read data into memory table
   
   for (int num = 0; num < 2; num++)
   {
      FILE* file;
      int ch;
      int row = 0, col = 0;
      
      asprintf(&filename, "%s.tv%d", extid, num+1);
      
      if (!(file = fopen(filename, "r")))
      {
         tell(0, "Error: Opening '%s' failed, status was '%s'", filename, strerror(errno));
         free(filename);
         return fail;
      }
      
      fseek(file, 0, SEEK_SET);
      
      for (int i = 0; i < 10; i++) 
      {
         row *= 10;
         row += fgetc(file) - '0';
      }
      
      while ((ch = fgetc(file)) != '\f')
      {
         col *= 10;
         col += ch - '0';
      }
      
      fseek(file, 40, SEEK_SET);                    // seek to data section
      
      for (int c = 0; c < col; c++)
      {
         Field header(file);
         const char* name = header.getString().c_str();
         
         for (int r = 0; r < row; r++)
         {
            Field field(file);
            
            if (field.isNumeric())
               table[r][name] = num2Str(field.getNumeric());
            else if (field.isString())
               table[r][name] = field.getXmlString();
         }
      }
      
      fclose(file);
      removeFile(filename);
      free(filename);
   }

   // -----------------------------------------------   
   // flush data to xml file

   std::string xmlBuf;
   
   xmlBuf += "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
   xmlBuf += "<!DOCTYPE Export>\n";
   xmlBuf += "<Export>\n";

   for (iter = table.begin(); iter != table.end(); iter++) 
   {
      xmlBuf += "  <Sendung>\n";

      for (std::map<std::string, std::string>::iterator i = iter->second.begin(); i != iter->second.end(); i++)
         xmlBuf += "    <" + i->first + ">" + i->second + "</" + i->first + ">\n";

      xmlBuf += "  </Sendung>\n";
   }
   
   xmlBuf += "</Export>\n";

   xmlData->size = xmlBuf.length()+TB;
   xmlData->memory = (char*)malloc(xmlData->size);
   strcpy(xmlData->memory, xmlBuf.c_str());

   asprintf(&filename, "%s.xml", extid);
   storeToFile(filename, xmlData->memory, xmlData->size);
   free(filename);

   return success;
}
