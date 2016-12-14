/* This file is part of RTags (http://rtags.net).

   RTags is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   RTags is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with RTags.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef IndexDataMessage_h
#define IndexDataMessage_h

#include "Diagnostic.h"
#include "IndexerJob.h"
#include "rct/Flags.h"
#include "rct/Serializer.h"
#include "rct/String.h"
#include "RTagsMessage.h"

class IndexDataMessage : public RTagsMessage
{
public:
    enum { MessageId = IndexDataMessageId };

    IndexDataMessage(const std::shared_ptr<IndexerJob> &job)
        : RTagsMessage(MessageId), mParseTime(0), mId(0), mIndexerJobFlags(job->flags), mBytesWritten(0)
    {}

    IndexDataMessage()
        : RTagsMessage(MessageId), mParseTime(0), mId(0), mBytesWritten(0)
    {}

    void encode(Serializer &serializer) const;
    void decode(Deserializer &deserializer);

    enum Flag {
        None = 0x0,
        ParseFailure = 0x1,
        InclusionError = 0x2,
        UsedPCH = 0x4
    };
    Flags<Flag> flags() const { return mFlags; }
    void setFlags(Flags<Flag> flags) { mFlags = flags; }
    void setFlag(Flag flag, bool on = true) { mFlags.set(flag, on); }

    Set<uint32_t> visitedFiles() const
    {
        Set<uint32_t> ret;
        for (const auto &it : mFiles) {
            if (it.second & Visited)
                ret.insert(it.first);
        }
        return ret;
    }

    Set<uint32_t> blockedFiles() const
    {
        Set<uint32_t> ret;
        for (const auto &it : mFiles) {
            if (!(it.second & Visited))
                ret.insert(it.first);
        }
        return ret;
    }

    const Path &project() const { return mProject; }
    void setProject(const Path &project) { mProject = project; }

    uint64_t id() const { return mId; }
    void setId(uint64_t id) { mId = id; }

    uint64_t parseTime() const { return mParseTime; }
    void setParseTime(uint64_t parseTime) { mParseTime = parseTime; }

    Flags<IndexerJob::Flag> indexerJobFlags() const { return mIndexerJobFlags; }
    void setIndexerJobFlags(Flags<IndexerJob::Flag> flags) { mIndexerJobFlags = flags; }

    uint32_t fileId() const { return mFileId; }
    void setFileId(uint32_t fileId) { mFileId = fileId; }

    const String &message() const { return mMessage; }
    void setMessage(const String &msg) { mMessage = msg; }

    FixIts &fixIts() { return mFixIts; }
    Diagnostics &diagnostics() { return mDiagnostics; }
    Includes &includes() { return mIncludes; }
    enum FileFlag {
        NoFileFlag = 0x0,
        Visited = 0x1,
        HeaderError = 0x2
    };
    Hash<uint32_t, Flags<FileFlag> > &files() { return mFiles; }
    const Hash<uint32_t, Flags<FileFlag> > &files() const { return mFiles; }

    size_t bytesWritten() const { return mBytesWritten; }
    void setBytesWritten(size_t bytesWritten) { mBytesWritten = bytesWritten; }

    SourceList sources() const { return mSources; }
    void setSources(const SourceList &sources) { mSources = sources; }
private:
    Path mProject;
    uint64_t mParseTime, mId;
    uint32_t mFileId;
    Flags<IndexerJob::Flag> mIndexerJobFlags; // indexerjobflags
    String mMessage; // used as output for dump when flags & Dump
    FixIts mFixIts;
    Diagnostics mDiagnostics;
    Includes mIncludes;
    Hash<uint32_t, Flags<FileFlag> > mFiles;
    Flags<Flag> mFlags;
    size_t mBytesWritten;
    SourceList mSources;
};

RCT_FLAGS(IndexDataMessage::Flag);
RCT_FLAGS(IndexDataMessage::FileFlag);

inline void IndexDataMessage::encode(Serializer &serializer) const
{
    serializer << mProject << mParseTime << mFileId << mId << mIndexerJobFlags << mMessage
               << mFixIts << mIncludes << mDiagnostics << mFiles << mFlags << mBytesWritten
               << static_cast<uint32_t>(mSources.size());
    for (const Source &source : mSources) {
        source.encode(serializer, Source::IgnoreSandbox);
    }
}

inline void IndexDataMessage::decode(Deserializer &deserializer)
{
    deserializer >> mProject >> mParseTime >> mFileId >> mId >> mIndexerJobFlags >> mMessage
                 >> mFixIts >> mIncludes >> mDiagnostics >> mFiles >> mFlags >> mBytesWritten;

    uint32_t size;
    deserializer >> size;
    mSources.resize(size);
    for (Source &source : mSources) {
        source.decode(deserializer, Source::IgnoreSandbox);
    }

}

#endif
