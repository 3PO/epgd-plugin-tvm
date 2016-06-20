/*
 * tvm.c
 *
 * See the README file for copyright information
 *
 */

#include <stdint.h>
#include <map>

#include "tvm.h"

//***************************************************************************
// Tvm
//***************************************************************************

Tvm2::Tvm2()
   : Plugin()
{
   pxsltStylesheet = 0;
   stmtMarkOldEvents = 0;
   stmtSetUpdFlgByFileref = 0;
   selectDistBySource = 0;
   selectId = 0;
   valueFileRef = 0;

   // config

   timeout = 30;
}

Tvm2::~Tvm2()
{
   delete stmtMarkOldEvents;
   delete stmtSetUpdFlgByFileref;
   delete selectDistBySource;
   delete valueFileRef;

   if (pxsltStylesheet)
      xsltFreeStylesheet(pxsltStylesheet);
}

int Tvm2::init(cEpgd* aObject, int aUtf8)
{
   Plugin::init(aObject, aUtf8);

   pxsltStylesheet = loadXSLT("tvmovie", confDir, utf8);

   return done;
}

int Tvm2::initDb()
{
   int status = success;

//    // --------
//    // update events set updflg = ?
//    //    where fileref = ? 
//    //      and source = ?
//    //      and updflg in (....)

//    stmtSetUpdFlgByFileref = new cDbStatement(obj->eventsDb);
   
//    stmtSetUpdFlgByFileref->build("update %s set ", obj->eventsDb->TableName());
//    stmtSetUpdFlgByFileref->bind("UpdFlg", cDbService::bndIn |cDbService:: bndSet);
//    stmtSetUpdFlgByFileref->build( " where ");
//    stmtSetUpdFlgByFileref->bind(valueFileRef, cDbService::bndIn |cDbService:: bndSet);
//    stmtSetUpdFlgByFileref->bind("Source", cDbService::bndIn | cDbService::bndSet, " and ");
//    stmtSetUpdFlgByFileref->build(" and updflg in (%s)", Us::getDeletable());

//    status += stmtSetUpdFlgByFileref->prepare();

//    // --------
//    // update events set delflg = ?, fileref = ?, updsp = ?
//    //    where fileref = ? 
//    //      and source = ?

//    valueFileRef = new cDbValue(obj->eventsDb->getField("FileRef"));
//    stmtSetDelByFileref = new cDbStatement(obj->eventsDb);
   
//    stmtSetDelByFileref->build("update %s set ", obj->eventsDb->TableName());
//    stmtSetDelByFileref->bind("DelFlg", cDbService::bndIn |cDbService:: bndSet);
//    stmtSetDelByFileref->bind("UpdFlg", cDbService::bndIn |cDbService:: bndSet, ", ");
//    stmtSetDelByFileref->bind("FileRef", cDbService::bndIn | cDbService::bndSet, ", ");
//    stmtSetDelByFileref->bind("UpdSp", cDbService::bndIn | cDbService::bndSet, ", ");
//    stmtSetDelByFileref->build( " where ");
//    stmtSetDelByFileref->bind(valueFileRef, cDbService::bndIn |cDbService:: bndSet);
//    stmtSetDelByFileref->bind("Source", cDbService::bndIn | cDbService::bndSet, " and ");

//    status += stmtSetDelByFileref->prepare();
  
   // ----------
   // update events 
   //   set updflg = case when updflg in (...) then 'D' else updflg end, 
   //       delflg = 'Y',
   //       updsp = unix_timestamp()
   //   where source = '...'
   //     and (source, fileref) not in (select source,fileref from fileref)

   stmtMarkOldEvents = new cDbStatement(obj->eventsDb);

   stmtMarkOldEvents->build("update %s set ", obj->eventsDb->TableName());
   stmtMarkOldEvents->build("updflg = case when updflg in (%s) then 'D' else updflg end, ", cEventState::getDeletable());
   stmtMarkOldEvents->build("delflg = 'Y', updsp = unix_timestamp()");
   stmtMarkOldEvents->build(" where source = '%s'", getSource());
   stmtMarkOldEvents->build(" and  (source, fileref) not in (select source,fileref from fileref)");

   status += stmtMarkOldEvents->prepare();  

   // ---------
   // select channelid, mergesp from channelmap 
   //     where source = ? and extid = ?

   selectId = new cDbStatement(obj->mapDb);

   selectId->build("select ");
   selectId->bind("ChannelId", cDBS::bndOut);
   selectId->bind("MergeSp", cDBS::bndOut, ", ");
   selectId->bind("Merge", cDBS::bndOut, ", ");
   selectId->build(" from %s where ", obj->mapDb->TableName());
   selectId->bind("Source", cDBS::bndIn | cDBS::bndSet);
   selectId->bind("ExternalId", cDBS::bndIn | cDBS::bndSet, " and ");
   
   status += selectId->prepare();

   // --------
   // select distinct extid from channelmap
   //   where source = ?
   
   selectDistBySource = new cDbStatement(obj->mapDb);
   selectDistBySource->build("select ");
   selectDistBySource->bind("ExternalId", cDBS::bndOut, "distinct ");
   selectDistBySource->build(" from %s where ", obj->mapDb->TableName());
   selectDistBySource->bind("Source", cDBS::bndIn | cDBS::bndSet);

   status += selectDistBySource->prepare();

   return status;
}

int Tvm2::exitDb()
{
   delete valueFileRef;           valueFileRef = 0;
   delete stmtMarkOldEvents;      stmtMarkOldEvents = 0;
   delete stmtSetUpdFlgByFileref; stmtSetUpdFlgByFileref = 0;
   delete selectDistBySource;     selectDistBySource = 0;
   delete selectId;               selectId = 0;

   return success;
}

//***************************************************************************
// At Config Item
//***************************************************************************

int Tvm2::atConfigItem(const char* Name, const char* Value)
{
   if (!strcasecmp(Name, "Timeout"))  
      timeout = atoi(Value);
   else 
      return fail;

   return success;
}

//***************************************************************************
// Ready
//***************************************************************************

int Tvm2::ready() 
{ 
   static int count = na;

   if (count == na)
   {
      char* where;

      asprintf(&where, "source = '%s'", getSource());

      if (obj->mapDb->countWhere(where, count) != success)
         count = na;

      free(where);
   }

   return count > 0;
}

//***************************************************************************
// Process 
// - Datei von TVM holen (sofern aktualisiert, noch nicht geholt oder fullupdate)
//    Erkennung der Aktualisierung erfolgt über die 'fileref' Tabelle
// - XSLT Konvertierung durchführen
// - Resultat je zugeordnetem Kanal in der 'events' 
//    Tabelle speichern/ aktualisieren incl. der 'fileref'
// - Neue 'fileref' in die 'fileref' Tabelle übernehmen 
//    -> damit ist die Verarbeitung quittiert
//***************************************************************************

int Tvm2::processDay(int day, int fullupdate, Statistic* stat)
{
   int status;
   MemoryStruct data;

   if (day != 0)
   {
      tell(1, "Skipping day %d for TVM plugin, since all days ar performed on day 0", day);
      return success;
   }
      
   obj->connection->startTransaction();

   // loop over all extid's of channelmap
   
   obj->mapDb->clear();
   obj->mapDb->setValue("Source", getSource());
   
   for (int res = selectDistBySource->find(); res && !obj->doShutDown(); 
        res = selectDistBySource->fetch())
   {
      char* extid = strdup(obj->mapDb->getStrValue("ExternalId"));
      int found;
      char* url;
      char* filename;      // the file to be downloaded
      int fileSize = 0;

      tell(0, "Checking tvm id %s", extid);

      // URL - Aufbau
      //    http://wwwa.tvmovie.de/static/tvghost/html/onlinedata/cftv520/tvdaten-premium-EXTID.cftv

      asprintf(&filename, "tvdaten-premium-%s.cftv", extid);    
      asprintf(&url, "http://wwwa.tvmovie.de/static/tvghost/html/onlinedata/cftv520/%s", filename);

      // lookup file information
         
      obj->fileDb->setValue("Name", filename);
      obj->fileDb->setValue("Source", getSource());
      found = obj->fileDb->find();

      // Don't check the day or the EPG2VDRConfig.upddays
      //   since TVM deliver all days in one file ...

      char* fileRef = 0;

      // first get the http header
         
      data.clear();
      data.headerOnly = yes;
         
      status = obj->downloadFile(url, fileSize, &data, 30, userAgent());
         
      if (status != success)
      {
         tell(0, "Download header of '%s' failed", url);
         free(filename);
         free(url);
         obj->fileDb->reset();
         free(extid);
         continue;
      }

      asprintf(&fileRef, "%s-%s", filename, data.tag);

      if (found && obj->fileDb->hasValue("FileRef", fileRef))
      {
         tell(2, "Skipping download of id %s due to non-update", extid);

         stat->nonUpdates++;
            
         free(filename);
         free(url);
         free(fileRef);
         obj->fileDb->reset();
         free(extid);
         continue;
      }

      data.clear();

      if (obj->downloadFile(url, fileSize, &data, timeout, userAgent()) != success || !zipValid(&data))
      {
         if (!zipValid(&data))
            tell(1, "Error, zip file '%s' not valid, check the coresponding tvm id in youre configuration", url);
         else
            tell(1, "Error, download of '%s' failed", url);

         free(filename);
         free(url);
         free(fileRef);
         obj->fileDb->reset();
         free(extid);
         continue;
      }

      free(fileRef);

      downloadImageFile(extid);

      if (fileSize > 0)
      {
         char* fileRef = 0;

         stat->bytes += fileSize;
         stat->files++;
            
         tell(0, "Downloaded file '%s' with (%d) Bytes", url, fileSize);

         // store to FS
         {            
            char* tmp = 0;
            asprintf(&tmp, "%s.zip", filename);
            obj->storeToFs(&data, tmp, getSource());
            free(tmp);
         }

         asprintf(&fileRef, "%s-%s", obj->fileDb->getStrValue("Name"), data.tag);
            
         if ((status = processFile(extid, &data, filename, fileRef)) != success)
         {
            tell(0, "Processing of '%s' failed", filename);
            status = fail;
            stat->rejected++;
         }

         if (status == success)
         {
// we can use this code instead of "stmtMarkOldEvents" !!
            
//             if (found)
//             {
//                // mark 'old' entrys in events table as deleted
//                // and 'fake' fileref to new to avoid deletion at cleanup

//                // first - set updflg to 'D' (only ift updflg in Us::getDeletable())

//                obj->eventsDb->clear();
//                obj->eventsDb->setValue("UpdFlg", "D");

//                // where ..

//                obj->eventsDb->setValue("Source", getSource());
//                valueFileRef->setValue(obj->fileDb->getStrValue("FileRef"));  // old fileref

//                stmtSetUpdFlgByFileref->execute();

//                // second - set delflg and fileref ..

//                obj->eventsDb->clear();
//                obj->eventsDb->setValue("DelFlg", "Y");
//                obj->eventsDb->setValue("FileRef", fileRef);                    // new fileref
//                obj->eventsDb->setValue("UpdSp", time(0));

//                // where ..

//                obj->eventsDb->setValue("Source", getSource());
//                valueFileRef->setValue(obj->fileDb->getStrValue("FileRef"));  // old fileref

//                stmtSetDelByFileref->execute();
//             }
               
            // Confirm processing of file
               
            obj->fileDb->setValue("ExternalId", extid);
            obj->fileDb->setValue("Tag", data.tag);
            obj->fileDb->setValue("FileRef", fileRef);
            obj->fileDb->store();

            obj->connection->commit();
            usleep(100000);
            obj->connection->startTransaction();
         }
            
         free(fileRef);
      }

      obj->fileDb->reset();
      free(url);
      free(filename);
      free(extid);
      data.clear();

      if (!obj->dbConnected())
         return fail;
   }

   selectDistBySource->freeResult();
   obj->connection->commit();

   return success;
}

//***************************************************************************
// Zip Valid
//***************************************************************************

int Tvm2::zipValid(MemoryStruct* data)
{
   char head[100+TB];

   // check content

   if (!data || !data->memory)
      return no;

   strncpy(head, data->memory, 100);
   head[100] = 0;   

   if (strcasestr(head, "DOCTYPE HTML"))
      return no;

   return yes;
}

//***************************************************************************
// Process Tvm File
//***************************************************************************

int Tvm2::processFile(const char* extid, MemoryStruct* data, 
                      const char* fileName, const char* fileRef)
{
   xmlDocPtr transformedDoc;
   xmlNodePtr xmlRoot;
   MemoryStruct xmlData;
   int count = 0;
   char* command;
   char password[7+TB];
   char* tmp = 0;

   memset(password, 0xb7, 7);
   password[7] = 0;

   tell(0, "Extracting '%s'", fileName);

   asprintf(&command, "unzip -o -qq -P %s -d %s/%s %s/%s/%s",
            password, EpgdConfig.cachePath, getSource(), 
            EpgdConfig.cachePath, getSource(), fileName);

   asprintf(&tmp, "%s/%s/%s.tv1", EpgdConfig.cachePath, getSource(), extid);

   if (system(command) < 0 || !fileExists(tmp))
   {
      tell(0, "Extracting of '%s' failed, missing at least '%s'", fileName, tmp);
      free(tmp);
      free(command);
      return fail;
   }

   free(tmp);
   free(command);

   if (createXml(extid, &xmlData) != success)
      return fail;

   if ((transformedDoc = obj->transformXml(xmlData.memory, xmlData.size, pxsltStylesheet, fileName)) == 0)
   {
      xmlData.clear();
      tell(0, "XSLT transformation for '%s' failed, ignoring", fileName);
      return fail;
   }

   xmlData.clear();

   if (!(xmlRoot = xmlDocGetRootElement(transformedDoc)))
   {
      tell(0, "Invalid xml document returned from xslt for '%s', ignoring", fileName);
      return fail;
   }

   // process 'all' events for 'all' configured channles of this extid

   obj->mapDb->clear();
   obj->mapDb->setValue("ExternalId", extid);
   obj->mapDb->setValue("Source", getSource());

   for (int f = selectId->find(); f && obj->dbConnected(); f = selectId->fetch())
   {
      const char* channelId = obj->mapDb->getStrValue("ChannelId");

      for (xmlNodePtr node = xmlRoot->xmlChildrenNode; node && obj->dbConnected(); node = node->next)
      {
         int insert;
         char* prop = 0;
         int id;

         // skip all unexpected elements
         
         if (node->type != XML_ELEMENT_NODE || strcmp((char*)node->name, "event") != 0)
            continue;
         
         // get/check id
         
         if (!(prop = (char*)xmlGetProp(node, (xmlChar*)"id")) || !*prop || !(id = atoi(prop)))
         {
            xmlFree(prop);
            tell(0, "Missing event id, ignoring!");
            continue;
         }
         
         xmlFree(prop);
         
         // create event ..
         
         obj->eventsDb->clear();
         obj->eventsDb->setValue("EventId", id);
         obj->eventsDb->setValue("ChannelId", channelId);
         
         insert = !obj->eventsDb->find();

         obj->eventsDb->setValue("Source", getSource());
         obj->eventsDb->setValue("FileRef", fileRef);

         // auto parse and set other fields

         obj->parseEvent(obj->eventsDb->getRow(), node);

         // ...
         
         time_t mergesp = obj->mapDb->getIntValue("MERGESP");
         long starttime = obj->eventsDb->getIntValue("STARTTIME");
         int merge = obj->mapDb->getIntValue("MERGE");

         // store ..
         
         if (insert)
         {
            // handle insert

            obj->eventsDb->setValue("VERSION", 0xFF);
            obj->eventsDb->setValue("TABLEID", 0L);
            obj->eventsDb->setValue("USEID", 0L);

            if (starttime <= mergesp)
               obj->eventsDb->setCharValue("UPDFLG", cEventState::usInactive);
            else
               obj->eventsDb->setCharValue("UPDFLG", merge > 1 ? cEventState::usMergeSpare : cEventState::usActive);

            obj->eventsDb->insert();
         }
         else
         {
            if (obj->eventsDb->hasValue("DELFLG", "Y"))
               obj->eventsDb->setValue("DELFLG", "N");

            if (obj->eventsDb->hasValue("UPDFLG", "D"))
            {
               if (starttime <= mergesp)
                  obj->eventsDb->setCharValue("UPDFLG", cEventState::usInactive);
               else
                  obj->eventsDb->setCharValue("UPDFLG", merge > 1 ? cEventState::usMergeSpare : cEventState::usActive);
            }

            obj->eventsDb->update();
         }

         obj->eventsDb->reset();
         count++;
      }
   }

   selectId->freeResult();

   xmlFreeDoc(transformedDoc);

   tell(2, "XML File '%s' processed, updated %d events", fileName, count);

   return success;
}

//***************************************************************************
// Cleanup After
//***************************************************************************

int Tvm2::cleanupAfter()
{
   // mark wasted events (delflg, ...)

   stmtMarkOldEvents->execute();   

   return success;
}

//***************************************************************************
// Download Image File
//***************************************************************************

int Tvm2::downloadImageFile(const char* extid)
{
   char* filename;
   int fileSize = 0;
   char* url;
   MemoryStruct data;
   char* command;
   char password[7+TB];
   int status;

   asprintf(&filename, "tvbilder-premium-%s.cftv", extid);
   asprintf(&url, "http://wwwa.tvmovie.de/static/tvghost/html/onlinedata/cftv520/%s", filename);

   if (obj->downloadFile(url, fileSize, &data, 30, userAgent()) != success)
   {
      tell(0, "Download at '%s' failed", url);
      free(url);
      free(filename);
      return fail;
   }

   free(url);
   status = obj->storeToFs(&data, filename, getSource());
 
   memset(password, 0xb7, 7);
   password[7] = 0;

   asprintf(&command, "unzip -o -qq -P %s -d %s/%s %s/%s/%s",
            password, EpgdConfig.cachePath, getSource(), 
            EpgdConfig.cachePath, getSource(), filename);

   if (system(command) < 0)
   {
      tell(0, "Extracting of '%s' failed", filename);
      free(command);
      free(filename);
      return fail;
   }

   // remove zipped file

   char* tmp = 0;
   asprintf(&tmp, "%s/%s/%s", EpgdConfig.cachePath, getSource(), filename);
   removeFile(tmp);
   free(tmp);
   
   free(command);
   free(filename);

   return status;
}

//***************************************************************************
// Get Picture
//***************************************************************************

int Tvm2::getPicture(const char* imagename, const char* fileRef, MemoryStruct* data)
{
   data->clear();
   obj->loadFromFs(data, imagename, getSource());

   // remove image after epgd has requested it (stored in table)

   char* tmp = 0;
   asprintf(&tmp, "%s/%s/%s", EpgdConfig.cachePath, getSource(), imagename);
   removeFile(tmp);
   free(tmp);

   return data->size;
}

//***************************************************************************
// Create Xml
//***************************************************************************

int Tvm2::createXml(const char* extid, MemoryStruct* xmlData)
{
   char* filename;
   map<int, map<string, string> > table;
   map<int, map<string, string> >::iterator iter;
   
   tell(0, "Creating xml of %s", extid);  

   // -----------------------------------------------
   // read data into memory table
   
   for (int num = 0; num < 2; num++)
   {
      FILE* file;
      int ch;
      int row = 0, col = 0;
      
      asprintf(&filename, "/%s/%s/%s.tv%d", EpgdConfig.cachePath, getSource(), extid, num+1);
      
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

   string xmlBuf;
   
   xmlBuf += "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
   xmlBuf += "<!DOCTYPE Export>\n";
   xmlBuf += "<Export>\n";

   for (iter = table.begin(); iter != table.end(); iter++) 
   {
      xmlBuf += "  <Sendung>\n";

      for (map<string, string>::iterator i = iter->second.begin(); i != iter->second.end(); i++)
         xmlBuf += "    <" + i->first + ">" + i->second + "</" + i->first + ">\n";

      xmlBuf += "  </Sendung>\n";
   }
   
   xmlBuf += "</Export>\n";

   xmlData->size = xmlBuf.length()+TB;
   xmlData->memory = (char*)malloc(xmlData->size);
   strcpy(xmlData->memory, xmlBuf.c_str());

   if (EpgdConfig.storeXmlToFs)
   {
      asprintf(&filename, "%s.xml", extid);
      obj->storeToFs(xmlData, filename, getSource());
      free(filename);
   }

   return success;
}

//***************************************************************************

extern "C" void* EPGPluginCreator () { return new Tvm2(); }
