/*
    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2001 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)
    Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include "Cache.h"

#include "CachedCSSStyleSheet.h"
#include "CachedFont.h"
#include "CachedImage.h"
#include "CachedScript.h"
#include "CachedXSLStyleSheet.h"
#include "DocLoader.h"
#include "Document.h"
#include "FrameLoader.h"
#include "FrameLoaderTypes.h"
#include "FrameView.h"
#include "Image.h"
#include "ResourceHandle.h"
#include "SecurityOrigin.h"
#include <stdio.h>
#include <wtf/CurrentTime.h>

// SSLAB : Including additional headers.
#include <CachedResource.h>
#include <sys/time.h>
#include "CString.h"
#include <QThread>
#include "SharedBuffer.h"
#include <QFile>

// SSLAB : Macro for debug messaging.
# define DPRINT(x); if(boolDebugging) { printf x ; }

using namespace std;

namespace WebCore {

static const int cDefaultCacheCapacity = 8192*1024; //256*1024*1024*1024;
static const double cMinDelayBeforeLiveDecodedPrune = 1; // Seconds.
static const float cTargetPrunePercentage = .95f; // Percentage of capacity toward which we prune, to avoid immediately pruning again.
static const double cDefaultDecodedDataDeletionInterval = 0;

Cache* cache()
{
    static Cache* staticCache = new Cache;
    return staticCache;
}

Cache::Cache()
    : m_disabled(false)
    , m_pruneEnabled(true)
    , m_inPruneDeadResources(false)
    , m_capacity(cDefaultCacheCapacity)
    , m_minDeadCapacity(0)
    , m_maxDeadCapacity(cDefaultCacheCapacity)
   , m_deadDecodedDataDeletionInterval(cDefaultDecodedDataDeletionInterval)
    , m_liveSize(0)
    , m_deadSize(0)
    // SSLAB : Initialize flags that toggled by signals from the browserMainWindow.
    , boolLoadable(true)
    , boolDebugging(false)
    , boolPCaching(false)
    , boolTracing(false)
{
    // SSLAB : Initialize load type stamp module.
    initLoadStamp();
}

static CachedResource* createResource(CachedResource::Type type, const KURL& url, const String& charset)
{
    switch (type) {
    case CachedResource::ImageResource:
        return new CachedImage(url.string());
    case CachedResource::CSSStyleSheet:
        return new CachedCSSStyleSheet(url.string(), charset);
    case CachedResource::Script:
        return new CachedScript(url.string(), charset);
    case CachedResource::FontResource:
        return new CachedFont(url.string());
#if ENABLE(XSLT)
    case CachedResource::XSLStyleSheet:
        return new CachedXSLStyleSheet(url.string());
#endif
#if ENABLE(XBL)
    case CachedResource::XBLStyleSheet:
        return new CachedXBLDocument(url.string());
#endif
    default:
        break;
    }

    return 0;
}

// SSLAB : Listing all cached resources.
void Cache::customStats() 
{
    CachedResourceMap::iterator e = m_resources.end();

    for (CachedResourceMap::iterator i = m_resources.begin(); i != e; ++i) {
        CachedResource* resource = i->second;

        // SSLAB : dump decoded resources (not yet implemented restoring these dumped resources).
        if(resource->decodedSize()) {   
            DPRINT(("SSLAB : Cache : customStats : %u \t| e %d \t| d %d \t| %s \n", 
                   RSHash(resource->url().utf8().data(), resource->url().length()),
                   resource->encodedSize(),
                   resource->decodedSize(),
                   resource->url().utf8().data()));

            dumpContents(resource);
        }

        DPRINT(("SSLAB : Cache : customStats : %p : %s \n",resource, resource->url().utf8().data()));        
    }
}

// SSLAB : Function for debugging fuction "dumpContents".
void Cache::dumpAgain(CachedResource *resource)
{
    unsigned offset = 0;
    unsigned moreDataLength;
    const char* moreData;
    FILE *fp;

    SharedBuffer *s = NULL;
    s = resource->data();

    if(!s) {
        DPRINT(("SSLAB : Cache : dumpAgain : Resource is exist but sharedBuffer.\n"));
        return;
    }
 
    fp = fopen("test2.bin", "wb");

     while(moreDataLength = s->getSomeData(moreData, offset)) {
        fwrite((void*)moreData, moreDataLength, 1, fp);
        offset += moreDataLength;
    }

    fclose(fp);

    DPRINT(("SSLAB : Cache : dumpAgain : Successfully dumped the resource... %s\n", resource->url().utf8().data()));
}

// SSLAB : Prototype function of the "restoreContentsV2".
void Cache::restoreContents(CachedResource::Type type, const char *fname, const char *target, const char *charset) {

    unsigned readSize;
    char buf[segmentSize];
    FILE *fp = NULL;

    DPRINT(("SSLAB : restoreContents : %s\n", target));

    KURL url(ParsedURLString, target);
    CachedResource* resource = resourceForURL(url);

    if(!resource) {
        resource = createResource(type, url, String(charset));
        ASSERT(resource);
        resource->setInCache(true);

        RefPtr<SharedBuffer> s = SharedBuffer::create();

        fp = fopen(fname, "rb");

	if(!fp) { return; }


        while(0 < (readSize = fread(buf, 1, segmentSize, fp))) { 
	    s->append(buf, (readSize < segmentSize) ? readSize : segmentSize);
        } 
        fclose(fp);

	resource->data(s, 1);

        if(type == CachedResource::ImageResource) 	((CachedImage*)resource)->checkNotify();
        else if(type == CachedResource::CSSStyleSheet) 	((CachedCSSStyleSheet*)resource)->checkNotify();
        else if(type == CachedResource::FontResource) 	((CachedFont*)resource)->checkNotify();
        else if(type == CachedResource::Script) 	((CachedScript*)resource)->checkNotify();

        if (!disabled())
            m_resources.set(url.string(), resource);  

	resourceAccessed(resource);
//	insertInLRUList(resource); // Object?
    }   
} 

// SSLAB : Initialize load type stamp module.
void Cache::initLoadStamp() {
    QFile loadStampFile(QLatin1String("/home/sslab/loadStamp.csv"));
    if(!loadStampFile.open(QFile::WriteOnly)) {
        printf("[TRACING] loadStampFile open error in %s.\n", __func__);
        exit(-1);
    }
    
    // SSLAB : Append Excel header strings. 
    char str_csvHeader[4096];
    sprintf(str_csvHeader, "\"Load type\",\"Load time\",.\"URL\"\n");
    loadStampFile.write(str_csvHeader, strlen(str_csvHeader));

    loadStampFile.close();
}

// SSLAB : Stamp load type of a resource on a file.
void Cache::loadStamp(double loadtime, const String& url) {

    char buff[4096] = {0,};

    QFile loadStampFile(QLatin1String("/home/sslab/loadStamp.csv"));                         
    if(!loadStampFile.open(QFile::Append)) {
        printf("[TRACING] loadStampFile open error in %s\n", __func__);
        exit(-1);
    }

    memset(buff, '\0', sizeof(char) * 4096);

    // SSLAB : Stamp load type of the resource.
    if(!valid) {
        sprintf(buff, "\"Cached, Invalid\",\"%f\",\"%s\"\n", loadtime, url.utf8().length() > 3500 ? NULL : url.utf8().data());
        printf("SSLAB : Cache : loadStamp : Cached & InValid\n");
    }
    else if(cached) {
        sprintf(buff, "\"Cached, Valid\",\"%f\",\"%s\"\n", loadtime, url.utf8().length() > 3500 ? NULL : url.utf8().data());
        printf("SSLAB : Cache : loadStamp : Cached & Valid\n");
    }
    else {
        sprintf(buff, "\"Not Cached\",\"%f\",\"%s\"\n", loadtime, url.utf8().length() > 3500 ? NULL : url.utf8().data());
        printf("SSLAB : Cache : loadStamp : Not Cached\n");
    }
    
    loadStampFile.write(buff, strlen(buff));

    loadStampFile.close();
}

// SSLAB : Hash module for converting url to hash value.
unsigned int Cache::RSHash(const char* str, unsigned int len)
{
   unsigned int b= 378551;
   unsigned int a = 63689;
   unsigned int hash = 0, i = 0;

   for(i = 0; i < len; str++, i++) {
      hash = hash * a + (*str);
      a = a * b;
   }

   return hash;
}

// SSLAB : Dump contents of an resource.
void Cache::dumpContents(CachedResource* resource)
{
    unsigned offset = 0;
    unsigned moreDataLength;
    const char* moreData;
    FILE *fp;
    char path[40];

    // SSLAB : Assemble path using RSHash value of the resource.

if(resource->type() == CachedResource::ImageResource)
    sprintf(path, "%si%u", NVRAM_PATH, RSHash(resource->url().utf8().data(), resource->url().length()));
else if(resource->type() == CachedResource::CSSStyleSheet) 
    sprintf(path, "%sc%u", NVRAM_PATH, RSHash(resource->url().utf8().data(), resource->url().length()));
else if(resource->type() == CachedResource::FontResource)
    sprintf(path, "%sf%u", NVRAM_PATH, RSHash(resource->url().utf8().data(), resource->url().length()));
else if(resource->type() == CachedResource::Script) 
    sprintf(path, "%sj%u", NVRAM_PATH, RSHash(resource->url().utf8().data(), resource->url().length()));

    DPRINT(("SSLAB : Cache : RSHash : %u\n", RSHash(resource->url().utf8().data(), resource->url().length())));

    // SSLAB : Get SharedBuffer of the resource.
    SharedBuffer *s = NULL;
    s = resource->data();
    if(!s) return;

    // SSLAB : Open file using already assembled path.
    fp = fopen(path, "wb");
 
    // SSLAB : Store headers which contains validity of the resource.
    fwrite((resource->response().httpHeaderField("age")).utf8().data(), 100, 1, fp);
    fwrite((resource->response().httpHeaderField("cache-control")).utf8().data(), 100, 1, fp);
    fwrite((resource->response().httpHeaderField("date")).utf8().data(), 100, 1, fp);
    fwrite((resource->response().httpHeaderField("expires")).utf8().data(), 100, 1, fp);
    fwrite((resource->response().httpHeaderField("last-modified")).utf8().data(), 100, 1, fp);
    fwrite((resource->response().httpHeaderField("pragma")).utf8().data(), 100, 1, fp);
 
    // SSLAB : Load contents from the SharedBuffer and Put it into the file.
    while(moreDataLength = s->getSomeData(moreData, offset)) {
        fwrite((void*)moreData, moreDataLength, 1, fp);       
        offset += moreDataLength;
    }

    fclose(fp);

    // SSLAB : Dumped successfully.
    DPRINT(("SSLAB : Cache : dumpContents : Successfully dumped the resource... %s\n", resource->url().utf8().data()));

/*  // SSLAB : ImageDecoder::copyFromSharedBuffer 분석.
    // SSLAB : offset부터 ?크기를 moreData에 받는다.
    while (unsigned moreDataLength = sharedBuffer.getSomeData(moreData, offset)) {
        // SSLAB : bufferLength : 받고자 하는 크기.
        // SSLAB : bytesExtracted : 이미 받은 크기.
        // SSLAB : bufferLength - bytesExtracted : 받아야 되는 크기.
        // SSLAB : byteToCopy : min을 이용해서 moreData 중 복사해야하는 크기 계산.
        unsigned bytesToCopy = min(bufferLength - bytesExtracted, moreDataLength);

        // SSLAB : byteToCopy만큼 moreData를 buffer에 누적 복사.
        memcpy(buffer + bytesExtracted, moreData, bytesToCopy);

        // SSLAB : 복사량 누적.
        bytesExtracted += bytesToCopy;

        // SSLAB : 복사 다했으면 종료.
        if (bytesExtracted == bufferLength)
            break;
            offset += bytesToCopy;
    }
    // SSLAB : 최종 복사량 반환.
    return bytesExtracted;
*/

}

// SSLAB : Trying to restore the requested resource from NVRAM.
CachedResource* Cache::restoreContentsV2(CachedResource::Type type, const char *target, const char *charset) {

    unsigned readSize;
    char buf[segmentSize];
    FILE *fp = NULL;
    char path[40];

    // SSLAB : Assemble path for the requested resource using hash value of it.
//    sprintf(path, "%s%u", NVRAM_PATH, RSHash(target, strlen(target)));


if(type == CachedResource::ImageResource)                                                 
    sprintf(path, "%si%u", NVRAM_PATH, RSHash(target, strlen(target)));
else if(type == CachedResource::CSSStyleSheet)                                            
    sprintf(path, "%sc%u", NVRAM_PATH, RSHash(target, strlen(target)));
else if(type == CachedResource::FontResource)                                             
    sprintf(path, "%sf%u", NVRAM_PATH, RSHash(target, strlen(target)));                                                                                                     
else if(type == CachedResource::Script) 
    sprintf(path, "%sj%u", NVRAM_PATH, RSHash(target, strlen(target))); 



    // SSLAB : Restoring what?
    DPRINT(("SSLAB : Cache : restoreContents : restoring... %s : %s\n", path, target));

    // SSLAB : If the resource is exist, there is no need to restore the resource.
    KURL url(ParsedURLString, target);
    CachedResource* resource = resourceForURL(url);

    // SSLAB : If the resource does not exist in memory cache, try to restore of it from NVRAM.
    if(!resource) {

        // SSLAB : Open the file using already assembled path.
        fp = fopen(path, "rb");

	if(!fp) {
            // SSLAB : Return case 1. The requested resource is not cached in NVRAM.
            DPRINT(("SSLAB : Cache : restoreContents : Failed!!!\n"));
	    return NULL;
	}

        // SSLAB : Create an CachedResource & SharedBuffer.
        resource = createResource(type, url, String(charset));
        ASSERT(resource);
        resource->setInCache(true);

        RefPtr<SharedBuffer> s = SharedBuffer::create();

        // SSLAB : Restore response header for checking validity of the resource.
        ResourceResponse response;
        
        fread(buf, 1, 100, fp); 
        response.setHTTPHeaderField(AtomicString("age"), String(buf));
        fread(buf, 1, 100, fp); 
        response.setHTTPHeaderField(AtomicString("cache-control"), String(buf));
        fread(buf, 1, 100, fp); 
        response.setHTTPHeaderField(AtomicString("date"), String(buf));
        fread(buf, 1, 100, fp); 
        response.setHTTPHeaderField(AtomicString("expires"), String(buf));
        fread(buf, 1, 100, fp); 
        response.setHTTPHeaderField(AtomicString("last-modified"), String(buf));
        fread(buf, 1, 100, fp); 
        response.setHTTPHeaderField(AtomicString("pragma"), String(buf));

        // SSLAB : Connecting the restored response header to the resource.
        // SSLAB : Set "m_isNull" to false insides of the function "setURL".
        // SSLAB : "m_isNull" must be set to false because of the response header is not empty anymore.
        response.setURL(url);  
        resource->setResponse(response);
        resource->updateResponseAfterRevalidation(response);

        // SSLAB : Is the response header really restored?
//	qWarning("SSLAB : HEADER : age : %s  %f", (resource->response().httpHeaderField("age")).utf8().data(), (resource->response()).age());
//	qWarning("SSLAB : HEADER : cache-control : %s", (resource->response().httpHeaderField("cache-control")).utf8().data());
//	qWarning("SSLAB : HEADER : date : %s", (resource->response().httpHeaderField("date")).utf8().data());
//	qWarning("SSLAB : HEADER : expires : %s", (resource->response().httpHeaderField("expires")).utf8().data());
//	qWarning("SSLAB : HEADER : last-modified : %s", (resource->response().httpHeaderField("last-modified")).utf8().data());
//	qWarning("SSLAB : HEADER : pragma : %s", (resource->response().httpHeaderField("pragma")).utf8().data());

	// SSLAB : Load contents from the file and put it into the SharedBuffer.
        while(0 < (readSize = fread(buf, 1, segmentSize, fp))) { 
	    s->append(buf, (readSize < segmentSize) ? readSize : segmentSize);
        } 

        fclose(fp);

        // SSLAB : Complete the CachedResource by connecting the SharedBuffer to it.
	resource->data(s, 1);
	
        // SSLAB : Increase access count of the resource by 1 because of the resource is just restored.
        resource->increaseAccessCount();  

        // SSLAB : Whether the contents of the file is safely loaded into the SharedBuffer?
        // SSLAB : Raw data is stored safety.
//	dumpAgain(s); 

        // SSLAB : Send notify to Client.
        if(type == CachedResource::ImageResource) 	((CachedImage*)resource)->checkNotify();
        else if(type == CachedResource::CSSStyleSheet) 	((CachedCSSStyleSheet*)resource)->checkNotify();
        else if(type == CachedResource::FontResource) 	((CachedFont*)resource)->checkNotify();
        else if(type == CachedResource::Script) 	((CachedScript*)resource)->checkNotify();

        // SSLAB : Registrate the restored resource on hashmap.
        if (!disabled())
            m_resources.set(url.string(), resource);  

        // SSLAB : Do remaining jobs such as insertion to Memory cache.
//	resourceAccessed(resource);  

        // SSLAB : Counting & Accumulating restored resources size for experiment.
	restoreCount++;
	restoreSize = restoreSize + resource->encodedSize();
//	printf("SSLAB : restoreCount : %d / restoreSize : %d\t : %s\n", restoreCount, restoreSize, path);       

        // SSLAB : Specialized code for image.
        // SSLAB : If the code is not exist, load() is called in the function "DocLoader::requestImage" even if the resource is restored.
        //if(type == CachedResource::ImageResource) {
	    CachedImage* p = static_cast<CachedImage*>(resource);
            // SSLAB : No error.
            p->m_errorOccurred = false;
	    // SSLAB : Cached.
	    p->m_status = (CachedResource::Status)Cached;
            // SSLAB : Omitted loading process.
	    p->m_loading = false;
	//}

        // SSLAB : Return case 2. The requested resource is successfully restored from NVRAM.
        DPRINT(("SSLAB : Cache : restoreContents : Succeed!!!\n"));
        return resource;
    }   
    
    // SSLAB : Return case 3. The requested resource is already in Memory Cache.
    DPRINT(("SSLAB : Cache : restoreContents : Already restored!!!\n"));
    return NULL;
} 

CachedResource* Cache::requestResource(DocLoader* docLoader, CachedResource::Type type, const KURL& url, const String& charset, bool requestIsPreload)
{
    // SSLAB : Request what?
    DPRINT(("SSLAB : Cache : requestResource : resquesting... %s\n", url.string().utf8().data()));

    // FIXME: Do we really need to special-case an empty URL?
    // Would it be better to just go on with the cache code and let it fail later?
    if (url.isEmpty())
        return 0;

    // SSLAB : Look up the resource in our map.
    CachedResource* resource = resourceForURL(url.string());

    // SSLAB : If the resource is not cached, trying to restore the requested resources from NVRAM.
    if(!resource & boolPCaching) {
        resource = restoreContentsV2(type, url.string().utf8().data(), "?");
        // SSLAB : If the resource is restored, check validty of the resource again.
        if(resource) {
            printf("SSLAB : Cache : requestResource : === start 2nd validation checking... %s\n", url.string().utf8().data());
            docLoader->checkForReload(url);
            printf("SSLAB : Cache : requestResource : === end 2nd validation checking...\n");
            resource = NULL;
            resource = resourceForURL(url.string());
        }
    }

/*
    // SSLAB : Code for dumping CSS.
    if(resource && type == CachedResource::CSSStyleSheet) {
	printf("SSLAB : Charset of CSS : %s\n", charset.utf8().data());  // 'UTF-8' in case of CSS, '' in case of Font
        dumpContents(resource);
    }
*/

    if (resource && requestIsPreload && !resource->isPreloaded()) {
        printf("SSLAB : Cache : requestResource : returned (case 1).\n");
        return 0;
    }

    if (SecurityOrigin::restrictAccessToLocal() && !SecurityOrigin::canLoad(url, String(), docLoader->doc())) {
        Document* doc = docLoader->doc();
        if (doc && !requestIsPreload)
            FrameLoader::reportLocalLoadFailed(doc->frame(), url.string());
        printf("SSLAB : Cache : requestResource : returned (case 2).\n");
        return 0;
    }
    
    if (!resource) {

        // SSLAB : The resource is not cached.
        DPRINT(("SSLAB : Cache : requestResource : The resource is not exist.\n"));

        // The resource does not exist. Create it.
        resource = createResource(type, url, charset);
        ASSERT(resource);

        // Pretend the resource is in the cache, to prevent it from being deleted during the load() call.
        // FIXME: CachedResource should just use normal refcounting instead.
        resource->setInCache(true);
        
        // SSLAB : Toggle btw enable or disable of sub-resource loading.
        if(boolLoadable) {
            resource->load(docLoader);
	}

        if (resource->errorOccurred()) {
            // We don't support immediate loads, but we do support immediate failure.
            // In that case we should to delete the resource now and return 0 because otherwise
            // it would leak if no ref/deref was ever done on it.
            resource->setInCache(false);
            delete resource;
            return 0;
        }

        if (!disabled())
            m_resources.set(url.string(), resource);  // The size will be added in later once the resource is loaded and calls back to us with the new size.
        else {
            // Kick the resource out of the cache, because the cache is disabled.
            resource->setInCache(false);
            resource->setDocLoader(docLoader);
        }
    }
    else
        // SSLAB : The resource is cached already.
        DPRINT(("SSLAB : Cache : requestResource : The resource is exist.\n"));

    if (resource->type() != type)
        return 0;

    if (!disabled()) {
        // This will move the resource to the front of its LRU list and increase its access count.
        resourceAccessed(resource);
    }

    // SSLAB : Test code for forcing to return first cashed image for every image request.
//  if(!temp && type == CachedResource::ImageResource) { printf("Caching first image.\n"); temp = resource; k = 1; }
//  if(temp && type == CachedResource::ImageResource) { printf("Returning cached image.\n");  return temp; }
//  else return resource;
    
    return resource;
}
    
CachedCSSStyleSheet* Cache::requestUserCSSStyleSheet(DocLoader* docLoader, const String& url, const String& charset)
{
    CachedCSSStyleSheet* userSheet;
    if (CachedResource* existing = resourceForURL(url)) {
        if (existing->type() != CachedResource::CSSStyleSheet)
            return 0;
        userSheet = static_cast<CachedCSSStyleSheet*>(existing);
    } else {
        userSheet = new CachedCSSStyleSheet(url, charset);

        // Pretend the resource is in the cache, to prevent it from being deleted during the load() call.
        // FIXME: CachedResource should just use normal refcounting instead.
        userSheet->setInCache(true);
        // Don't load incrementally, skip load checks, don't send resource load callbacks.
        //userSheet->load(docLoader, false, SkipSecurityCheck, false);
        if (!disabled())
            m_resources.set(url, userSheet);
        else
            userSheet->setInCache(false);
    }

    if (!disabled()) {
        // This will move the resource to the front of its LRU list and increase its access count.
        resourceAccessed(userSheet);
    }

    return userSheet;
}
    
void Cache::revalidateResource(CachedResource* resource, DocLoader* docLoader)
{
    ASSERT(resource);
    ASSERT(resource->inCache());
    ASSERT(resource == m_resources.get(resource->url()));
    ASSERT(!disabled());
    if (resource->resourceToRevalidate())
        return;
    if (!resource->canUseCacheValidator()) {
        evict(resource);
        return;
    }
    const String& url = resource->url();
    CachedResource* newResource = createResource(resource->type(), KURL(ParsedURLString, url), resource->encoding());
    newResource->setResourceToRevalidate(resource);
    evict(resource);
    m_resources.set(url, newResource);
    newResource->setInCache(true);
    resourceAccessed(newResource);

    // SSLAB : Toggle btw enable or disable of sub-resource loading.
    if(boolLoadable) {
        newResource->load(docLoader);
    }
}
    
void Cache::revalidationSucceeded(CachedResource* revalidatingResource, const ResourceResponse& response)
{
    CachedResource* resource = revalidatingResource->resourceToRevalidate();
    ASSERT(resource);
    ASSERT(!resource->inCache());
    ASSERT(resource->isLoaded());
    ASSERT(revalidatingResource->inCache());
    
    evict(revalidatingResource);

    ASSERT(!m_resources.get(resource->url()));
    m_resources.set(resource->url(), resource);
    resource->setInCache(true);
    resource->updateResponseAfterRevalidation(response);
    insertInLRUList(resource);
    int delta = resource->size();
    if (resource->decodedSize() && resource->hasClients())
        insertInLiveDecodedResourcesList(resource);
    if (delta)
        adjustSize(resource->hasClients(), delta);
    
    revalidatingResource->switchClientsToRevalidatedResource();
    // this deletes the revalidating resource
    revalidatingResource->clearResourceToRevalidate();
}

void Cache::revalidationFailed(CachedResource* revalidatingResource)
{
    ASSERT(revalidatingResource->resourceToRevalidate());
    revalidatingResource->clearResourceToRevalidate();
}

CachedResource* Cache::resourceForURL(const String& url)
{
    CachedResource* resource = m_resources.get(url);
    if (resource && !resource->makePurgeable(false)) {
        ASSERT(!resource->hasClients());
        evict(resource);
        return 0;
    }
    return resource;
}

unsigned Cache::deadCapacity() const 
{
    // Dead resource capacity is whatever space is not occupied by live resources, bounded by an independent minimum and maximum.
    unsigned capacity = m_capacity - min(m_liveSize, m_capacity); // Start with available capacity.
    capacity = max(capacity, m_minDeadCapacity); // Make sure it's above the minimum.
    capacity = min(capacity, m_maxDeadCapacity); // Make sure it's below the maximum.
    return capacity;
}

unsigned Cache::liveCapacity() const 
{ 
    // Live resource capacity is whatever is left over after calculating dead resource capacity.
    return m_capacity - deadCapacity();
}

void Cache::pruneLiveResources()
{
    //qWarning("### %s:%d:%s\n", __FILE__,__LINE__,__func__);
    if (!m_pruneEnabled)
        return;

    unsigned capacity = liveCapacity();
    if (capacity && m_liveSize <= capacity)
        return;

    unsigned targetSize = static_cast<unsigned>(capacity * cTargetPrunePercentage); // Cut by a percentage to avoid immediately pruning again.
    double currentTime = FrameView::currentPaintTimeStamp();
    if (!currentTime) // In case prune is called directly, outside of a Frame paint.
        currentTime = WTF::currentTime();
    
    // Destroy any decoded data in live objects that we can.
    // Start from the tail, since this is the least recently accessed of the objects.

    // The list might not be sorted by the m_lastDecodedAccessTime. The impact
    // of this weaker invariant is minor as the below if statement to check the
    // elapsedTime will evaluate to false as the currentTime will be a lot
    // greater than the current->m_lastDecodedAccessTime.
    // For more details see: https://bugs.webkit.org/show_bug.cgi?id=30209
    CachedResource* current = m_liveDecodedResources.m_tail;
    while (current) {
        CachedResource* prev = current->m_prevInLiveResourcesList;
        ASSERT(current->hasClients());
        if (current->isLoaded() && current->decodedSize()) {
            // Check to see if the remaining resources are too new to prune.
            double elapsedTime = currentTime - current->m_lastDecodedAccessTime;
            if (elapsedTime < cMinDelayBeforeLiveDecodedPrune)
                return;

            // Destroy our decoded data. This will remove us from 
            // m_liveDecodedResources, and possibly move us to a different LRU 
            // list in m_allResources.
            current->destroyDecodedData();

            if (targetSize && m_liveSize <= targetSize)
                return;
        }
        current = prev;
    }
}

void Cache::pruneDeadResources()
{
    //qWarning("### %s:%d:%s\n", __FILE__,__LINE__,__func__);
    if (!m_pruneEnabled)
        return;

    unsigned capacity = deadCapacity();
    if (capacity && m_deadSize <= capacity)
        return;

    unsigned targetSize = static_cast<unsigned>(capacity * cTargetPrunePercentage); // Cut by a percentage to avoid immediately pruning again.
    int size = m_allResources.size();
    
    if (!m_inPruneDeadResources) {
        // See if we have any purged resources we can evict.
        for (int i = 0; i < size; i++) {
            CachedResource* current = m_allResources[i].m_tail;
            while (current) {
                CachedResource* prev = current->m_prevInAllResourcesList;
                if (current->wasPurged()) {
                    ASSERT(!current->hasClients());
                    ASSERT(!current->isPreloaded());
                    evict(current);
                }
                current = prev;
            }
        }
        if (targetSize && m_deadSize <= targetSize)
            return;
    }
    
    bool canShrinkLRULists = true;
    m_inPruneDeadResources = true;

    for (int i = size - 1; i >= 0; i--) {
        // Remove from the tail, since this is the least frequently accessed of the objects.
        CachedResource* current = m_allResources[i].m_tail;
        
        // First flush all the decoded data in this queue.
        while (current) {
            CachedResource* prev = current->m_prevInAllResourcesList;
            if (!current->hasClients() && !current->isPreloaded() && current->isLoaded()) {
                // Destroy our decoded data. This will remove us from 
                // m_liveDecodedResources, and possibly move us to a different 
                // LRU list in m_allResources.
                current->destroyDecodedData();
                
                if (targetSize && m_deadSize <= targetSize) {
                    m_inPruneDeadResources = false;
                    return;
                }
            }
            current = prev;
        }

        // Now evict objects from this queue.
        current = m_allResources[i].m_tail;
        while (current) {
            CachedResource* prev = current->m_prevInAllResourcesList;
            if (!current->hasClients() && !current->isPreloaded() && !current->isCacheValidator()) {
                evict(current);
                // If evict() caused pruneDeadResources() to be re-entered, bail out. This can happen when removing an
                // SVG CachedImage that has subresources.
                if (!m_inPruneDeadResources)
                    return;

                if (targetSize && m_deadSize <= targetSize) {
                    m_inPruneDeadResources = false;
                    return;
                }
            }
            current = prev;
        }
            
        // Shrink the vector back down so we don't waste time inspecting
        // empty LRU lists on future prunes.
        if (m_allResources[i].m_head)
            canShrinkLRULists = false;
        else if (canShrinkLRULists)
            m_allResources.resize(i);
    }
    m_inPruneDeadResources = false;
}

void Cache::setCapacities(unsigned minDeadBytes, unsigned maxDeadBytes, unsigned totalBytes)
{
    qWarning("### %s:%d:%s\n", __FILE__,__LINE__,__func__);
    ASSERT(minDeadBytes <= maxDeadBytes);
    ASSERT(maxDeadBytes <= totalBytes);
    m_minDeadCapacity = minDeadBytes;
    m_maxDeadCapacity = maxDeadBytes;
    m_capacity = totalBytes;
    prune();
}

void Cache::evict(CachedResource* resource)
{
    qWarning("### %s:%d:%s : %s\n", __FILE__,__LINE__,__func__,resource->url().utf8().data());
    // The resource may have already been removed by someone other than our caller,
    // who needed a fresh copy for a reload. See <http://bugs.webkit.org/show_bug.cgi?id=12479#c6>.
    if (resource->inCache()) {
        // Remove from the resource map.
        m_resources.remove(resource->url());
        resource->setInCache(false);

        // Remove from the appropriate LRU list.
        removeFromLRUList(resource);
        removeFromLiveDecodedResourcesList(resource);

        // Subtract from our size totals.
        int delta = -static_cast<int>(resource->size());
        if (delta)
            adjustSize(resource->hasClients(), delta);

        // SSLAB : Dump the resource with eviction.
        if(boolPCaching) dumpContents(resource);
    } else
        ASSERT(m_resources.get(resource->url()) != resource);

    if (resource->canDelete()) {

        qWarning("### %s:%d:%s DELETED! %s\n",__FILE__,__LINE__,__func__,resource->url().utf8().data());
        delete resource;
    }
}

void Cache::addDocLoader(DocLoader* docLoader)
{
    m_docLoaders.add(docLoader);
}

void Cache::removeDocLoader(DocLoader* docLoader)
{
    m_docLoaders.remove(docLoader);
}

static inline unsigned fastLog2(unsigned i)
{
    unsigned log2 = 0;
    if (i & (i - 1))
        log2 += 1;
    if (i >> 16)
        log2 += 16, i >>= 16;
    if (i >> 8)
        log2 += 8, i >>= 8;
    if (i >> 4)
        log2 += 4, i >>= 4;
    if (i >> 2)
        log2 += 2, i >>= 2;
    if (i >> 1)
        log2 += 1;
    return log2;
}

Cache::LRUList* Cache::lruListFor(CachedResource* resource)
{
    unsigned accessCount = max(resource->accessCount(), 1U);
    unsigned queueIndex = fastLog2(resource->size() / accessCount);
#ifndef NDEBUG
    resource->m_lruIndex = queueIndex;
#endif
    if (m_allResources.size() <= queueIndex)
        m_allResources.grow(queueIndex + 1);
    return &m_allResources[queueIndex];
}

void Cache::removeFromLRUList(CachedResource* resource)
{
    // If we've never been accessed, then we're brand new and not in any list.
    if (resource->accessCount() == 0) {
        return;
    }

#if !ASSERT_DISABLED
    unsigned oldListIndex = resource->m_lruIndex;
#endif

    LRUList* list = lruListFor(resource);

#if !ASSERT_DISABLED
    // Verify that the list we got is the list we want.
    ASSERT(resource->m_lruIndex == oldListIndex);

    // Verify that we are in fact in this list.
    bool found = false;
    for (CachedResource* current = list->m_head; current; current = current->m_nextInAllResourcesList) {
        if (current == resource) {
            found = true;
            break;
        }
    }
    ASSERT(found);
#endif

    CachedResource* next = resource->m_nextInAllResourcesList;
    CachedResource* prev = resource->m_prevInAllResourcesList;
    
    if (next == 0 && prev == 0 && list->m_head != resource)
        return;
    
    resource->m_nextInAllResourcesList = 0;
    resource->m_prevInAllResourcesList = 0;
    
    if (next)
        next->m_prevInAllResourcesList = prev;
    else if (list->m_tail == resource)
        list->m_tail = prev;

    if (prev)
        prev->m_nextInAllResourcesList = next;
    else if (list->m_head == resource)
        list->m_head = next;
}

void Cache::insertInLRUList(CachedResource* resource)
{
    // Make sure we aren't in some list already.
    ASSERT(!resource->m_nextInAllResourcesList && !resource->m_prevInAllResourcesList);
    ASSERT(resource->inCache());
    ASSERT(resource->accessCount() > 0);
    
    LRUList* list = lruListFor(resource);

    resource->m_nextInAllResourcesList = list->m_head;
    if (list->m_head)
        list->m_head->m_prevInAllResourcesList = resource;
    list->m_head = resource;
    
    if (!resource->m_nextInAllResourcesList)
        list->m_tail = resource;
        
#ifndef NDEBUG
    // Verify that we are in now in the list like we should be.
    list = lruListFor(resource);
    bool found = false;
    for (CachedResource* current = list->m_head; current; current = current->m_nextInAllResourcesList) {
        if (current == resource) {
            found = true;
            break;
        }
    }
    ASSERT(found);
#endif
}

void Cache::resourceAccessed(CachedResource* resource)
{
    ASSERT(resource->inCache());
    
    // Need to make sure to remove before we increase the access count, since
    // the queue will possibly change.
    removeFromLRUList(resource);
    
    // If this is the first time the resource has been accessed, adjust the size of the cache to account for its initial size.
    if (!resource->accessCount())
        adjustSize(resource->hasClients(), resource->size());
    
    // Add to our access count.
    resource->increaseAccessCount();
    
    // Now insert into the new queue.
    insertInLRUList(resource);
}

void Cache::removeFromLiveDecodedResourcesList(CachedResource* resource)
{
    // If we've never been accessed, then we're brand new and not in any list.
    if (!resource->m_inLiveDecodedResourcesList)
        return;
    resource->m_inLiveDecodedResourcesList = false;

#ifndef NDEBUG
    // Verify that we are in fact in this list.
    bool found = false;
    for (CachedResource* current = m_liveDecodedResources.m_head; current; current = current->m_nextInLiveResourcesList) {
        if (current == resource) {
            found = true;
            break;
        }
    }
    ASSERT(found);
#endif

    CachedResource* next = resource->m_nextInLiveResourcesList;
    CachedResource* prev = resource->m_prevInLiveResourcesList;
    
    if (next == 0 && prev == 0 && m_liveDecodedResources.m_head != resource)
        return;
    
    resource->m_nextInLiveResourcesList = 0;
    resource->m_prevInLiveResourcesList = 0;
    
    if (next)
        next->m_prevInLiveResourcesList = prev;
    else if (m_liveDecodedResources.m_tail == resource)
        m_liveDecodedResources.m_tail = prev;

    if (prev)
        prev->m_nextInLiveResourcesList = next;
    else if (m_liveDecodedResources.m_head == resource)
        m_liveDecodedResources.m_head = next;
}

void Cache::insertInLiveDecodedResourcesList(CachedResource* resource)
{
    // Make sure we aren't in the list already.
    ASSERT(!resource->m_nextInLiveResourcesList && !resource->m_prevInLiveResourcesList && !resource->m_inLiveDecodedResourcesList);
    resource->m_inLiveDecodedResourcesList = true;

    resource->m_nextInLiveResourcesList = m_liveDecodedResources.m_head;
    if (m_liveDecodedResources.m_head)
        m_liveDecodedResources.m_head->m_prevInLiveResourcesList = resource;
    m_liveDecodedResources.m_head = resource;
    
    if (!resource->m_nextInLiveResourcesList)
        m_liveDecodedResources.m_tail = resource;
        
#ifndef NDEBUG
    // Verify that we are in now in the list like we should be.
    bool found = false;
    for (CachedResource* current = m_liveDecodedResources.m_head; current; current = current->m_nextInLiveResourcesList) {
        if (current == resource) {
            found = true;
            break;
        }
    }
    ASSERT(found);
#endif

}

void Cache::addToLiveResourcesSize(CachedResource* resource)
{
    m_liveSize += resource->size();
    m_deadSize -= resource->size();
}

void Cache::removeFromLiveResourcesSize(CachedResource* resource)
{
    m_liveSize -= resource->size();
    m_deadSize += resource->size();
}

void Cache::adjustSize(bool live, int delta)
{
    if (live) {
        ASSERT(delta >= 0 || ((int)m_liveSize + delta >= 0));
        m_liveSize += delta;
    } else {
        ASSERT(delta >= 0 || ((int)m_deadSize + delta >= 0));
        m_deadSize += delta;
    }
}

void Cache::TypeStatistic::addResource(CachedResource* o)
{
    bool purged = o->wasPurged();
    bool purgeable = o->isPurgeable() && !purged; 
    int pageSize = (o->encodedSize() + o->overheadSize() + 4095) & ~4095;
    count++;
    size += purged ? 0 : o->size(); 
    liveSize += o->hasClients() ? o->size() : 0;
    decodedSize += o->decodedSize();
    purgeableSize += purgeable ? pageSize : 0;
    purgedSize += purged ? pageSize : 0;
}

Cache::Statistics Cache::getStatistics()
{
    Statistics stats;
    CachedResourceMap::iterator e = m_resources.end();
    for (CachedResourceMap::iterator i = m_resources.begin(); i != e; ++i) {
        CachedResource* resource = i->second;
        switch (resource->type()) {
        case CachedResource::ImageResource:
            stats.images.addResource(resource);
            break;
        case CachedResource::CSSStyleSheet:
            stats.cssStyleSheets.addResource(resource);
            break;
        case CachedResource::Script:
            stats.scripts.addResource(resource);
            break;
#if ENABLE(XSLT)
        case CachedResource::XSLStyleSheet:
            stats.xslStyleSheets.addResource(resource);
            break;
#endif
        case CachedResource::FontResource:
            stats.fonts.addResource(resource);
            break;
#if ENABLE(XBL)
        case CachedResource::XBL:
            stats.xblDocs.addResource(resource)
            break;
#endif
        default:
            break;
        }
    }
    return stats;
}

void Cache::setDisabled(bool disabled)
{
    m_disabled = disabled;
    if (!m_disabled)
        return;

    for (;;) {
        CachedResourceMap::iterator i = m_resources.begin();
        if (i == m_resources.end())
            break;
        evict(i->second);
    }
}

//#ifndef NDEBUG
void Cache::dumpStats()
{
    Statistics s = getStatistics();
    printf("%-11s %-11s %-11s %-11s %-11s %-11s %-11s\n", "", "Count", "Size", "LiveSize", "DecodedSize", "PurgeableSize", "PurgedSize");
    printf("%-11s %-11s %-11s %-11s %-11s %-11s %-11s\n", "-----------", "-----------", "-----------", "-----------", "-----------", "-----------", "-----------");
    printf("%-11s %11d %11d %11d %11d %11d %11d\n", "Images", s.images.count, s.images.size, s.images.liveSize, s.images.decodedSize, s.images.purgeableSize, s.images.purgedSize);
    printf("%-11s %11d %11d %11d %11d %11d %11d\n", "CSS", s.cssStyleSheets.count, s.cssStyleSheets.size, s.cssStyleSheets.liveSize, s.cssStyleSheets.decodedSize, s.cssStyleSheets.purgeableSize, s.cssStyleSheets.purgedSize);
#if ENABLE(XSLT)
    printf("%-11s %11d %11d %11d %11d %11d %11d\n", "XSL", s.xslStyleSheets.count, s.xslStyleSheets.size, s.xslStyleSheets.liveSize, s.xslStyleSheets.decodedSize, s.xslStyleSheets.purgeableSize, s.xslStyleSheets.purgedSize);
#endif
    printf("%-11s %11d %11d %11d %11d %11d %11d\n", "JavaScript", s.scripts.count, s.scripts.size, s.scripts.liveSize, s.scripts.decodedSize, s.scripts.purgeableSize, s.scripts.purgedSize);
    printf("%-11s %11d %11d %11d %11d %11d %11d\n", "Fonts", s.fonts.count, s.fonts.size, s.fonts.liveSize, s.fonts.decodedSize, s.fonts.purgeableSize, s.fonts.purgedSize);
    printf("%-11s %-11s %-11s %-11s %-11s %-11s %-11s\n\n", "-----------", "-----------", "-----------", "-----------", "-----------", "-----------", "-----------");
}

void Cache::dumpLRULists(bool includeLive) const
{
    printf("LRU-SP lists in eviction order (Kilobytes decoded, Kilobytes encoded, Access count, Referenced):\n");

    int size = m_allResources.size();
    for (int i = size - 1; i >= 0; i--) {
        printf("\n\nList %d: ", i);
        CachedResource* current = m_allResources[i].m_tail;
        while (current) {
            CachedResource* prev = current->m_prevInAllResourcesList;
            if (includeLive || !current->hasClients()) {
                printf("(%.1fK, %.1fK, %uA, %dR); ", current->decodedSize() / 1024.0f, (current->encodedSize() + current->overheadSize()) / 1024.0f, current->accessCount(), current->hasClients());

		printf("%s /", current->url().utf8().data());
	    }
            current = prev;
        }
    }
}
//#endif

} // namespace WebCore
