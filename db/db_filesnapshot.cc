// Copyright (c) 2012 Facebook.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>
#include <stdint.h>
#include "db/db_impl.h"
#include "db/filename.h"
#include "db/version_set.h"
#include "rocksdb/db.h"
#include "rocksdb/env.h"
#include "port/port.h"
#include "util/mutexlock.h"

namespace rocksdb {

Status DBImpl::DisableFileDeletions() {
  MutexLock l(&mutex_);
  disable_delete_obsolete_files_ = true;
  Log(options_.info_log, "File Deletions Disabled");
  return Status::OK();
}

Status DBImpl::EnableFileDeletions() {
  MutexLock l(&mutex_);
  disable_delete_obsolete_files_ = false;
  Log(options_.info_log, "File Deletions Enabled");
  return Status::OK();
}

Status DBImpl::GetLiveFiles(std::vector<std::string>& ret,
                            uint64_t* manifest_file_size,
                            bool flush_memtable) {

  *manifest_file_size = 0;

  if (flush_memtable) {
    // flush all dirty data to disk.
    Status status =  Flush(FlushOptions());
    if (!status.ok()) {
      Log(options_.info_log, "Cannot Flush data %s\n",
          status.ToString().c_str());
      return status;
    }
  }

  MutexLock l(&mutex_);

  // Make a set of all of the live *.sst files
  std::set<uint64_t> live;
  versions_->AddLiveFilesCurrentVersion(&live);

  ret.clear();
  ret.reserve(live.size() + 2); //*.sst + CURRENT + MANIFEST

  // create names of the live files. The names are not absolute
  // paths, instead they are relative to dbname_;
  for (auto live_file : live) {
    ret.push_back(TableFileName("", live_file));
  }

  ret.push_back(CurrentFileName(""));
  ret.push_back(DescriptorFileName("", versions_->ManifestFileNumber()));

  // find length of manifest file while holding the mutex lock
  *manifest_file_size = versions_->ManifestFileSize();

  return Status::OK();
}

Status DBImpl::GetSortedWalFiles(VectorLogPtr& files) {
  // First get sorted files in archive dir, then append sorted files from main
  // dir to maintain sorted order

  // list wal files in archive dir.
  Status s;
  std::string archivedir = ArchivalDirectory(dbname_);
  if (env_->FileExists(archivedir)) {
    s = AppendSortedWalsOfType(archivedir, files, kArchivedLogFile);
    if (!s.ok()) {
      return s;
    }
  }
  // list wal files in main db dir.
  return AppendSortedWalsOfType(dbname_, files, kAliveLogFile);
}

Status DBImpl::DeleteWalFiles(const VectorLogPtr& files) {
  Status s;
  std::string archivedir = ArchivalDirectory(dbname_);
  std::string files_not_deleted;
  for (const auto& wal : files) {
    /* Try deleting in the dir that pathname points to for the logfile.
       This may fail if we try to delete a log file which was live when captured
       but is archived now. Try deleting it from archive also
     */
    Status st = env_->DeleteFile(dbname_ + "/" + wal->PathName());
    if (!st.ok()) {
      if (wal->Type() == kAliveLogFile &&
          env_->DeleteFile(LogFileName(archivedir, wal->LogNumber())).ok()) {
        continue;
      }
      files_not_deleted.append(wal->PathName());
    }
  }
  if (!files_not_deleted.empty()) {
    return Status::IOError("Deleted all requested files except: " +
                           files_not_deleted);
  }
  return Status::OK();
}

}
