// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_ROCKSDB_UNIVERSAL_COMPACTION_OPTIONS_H
#define STORAGE_ROCKSDB_UNIVERSAL_COMPACTION_OPTIONS_H

#include <stddef.h>
#include <string>
#include <memory>
#include <vector>
#include <stdint.h>
#include <climits>
#include "rocksdb/slice.h"
#include "rocksdb/statistics.h"

namespace rocksdb {

//
// Algorithm used to make a compaction request stop picking new files
// into a single compaction run
//
enum CompactionStopStyle {
  kCompactionStopStyleSimilarSize, // pick files of similar size
  kCompactionStopStyleTotalSize    // total size of picked files > next file
};

class CompactionOptionsUniversal {
 public:

  // Percentage flexibilty while comparing file size. If the candidate file(s)
  // size is 1% smaller than the next file's size, then include next file into
  // this candidate set. // Default: 1
  unsigned int size_ratio;

  // The minimum number of files in a single compaction run. Default: 2
  unsigned int min_merge_width;

  // The maximum number of files in a single compaction run. Default: UINT_MAX
  unsigned int max_merge_width;

  // The size amplification is defined as the amount (in percentage) of
  // additional storage needed to store a single byte of data in the database.
  // For example, a size amplification of 2% means that a database that
  // contains 100 bytes of user-data may occupy upto 102 bytes of
  // physical storage. By this definition, a fully compacted database has
  // a size amplification of 0%. Rocksdb uses the following heuristic
  // to calculate size amplification: it assumes that all files excluding
  // the earliest file contribute to the size amplification.
  // Default: 200, which means that a 100 byte database could require upto
  // 300 bytes of storage.
  unsigned int max_size_amplification_percent;

  // The algorithm used to stop picking files into a single compaction run
  // Default: kCompactionStopStyleTotalSize
  CompactionStopStyle stop_style;

  // Default set of parameters
  CompactionOptionsUniversal() :
    size_ratio(1),
    min_merge_width(2),
    max_merge_width(UINT_MAX),
    max_size_amplification_percent(200),
    stop_style(kCompactionStopStyleTotalSize) {
  }
};

}  // namespace rocksdb

#include "rocksdb/rocksdb_to_leveldb.h"

#endif  // STORAGE_ROCKSDB_UNIVERSAL_COMPACTION_OPTIONS_H
