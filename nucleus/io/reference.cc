/*
 * Copyright 2018 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "nucleus/io/reference.h"

#include <algorithm>
#include <numeric>

#include "absl/strings/str_cat.h"
#include "tensorflow/core/platform/logging.h"

namespace nucleus {

using absl::StrCat;
using nucleus::genomics::v1::Range;

// ###########################################################################
//
// GenomeReference code
//
// ###########################################################################

int GenomeReference::NContigs() const { return Contigs().size(); }

std::vector<string> GenomeReference::ContigNames() const {
  const auto& contigs = Contigs();
  std::vector<string> keys;
  keys.reserve(contigs.size());
  for (const auto& contig : contigs) {
    keys.push_back(contig.name());
  }
  return keys;
}

bool GenomeReference::HasContig(const string& contig_name) const {
  const auto& contigs = Contigs();
  return std::any_of(contigs.cbegin(), contigs.cend(),
                     [&](const nucleus::genomics::v1::ContigInfo& contig) {
                       return contig.name() == contig_name;
                     });
}

StatusOr<const nucleus::genomics::v1::ContigInfo*> GenomeReference::Contig(
    const string& contig_name) const {
  for (const auto& contig : Contigs()) {
    if (contig.name() == contig_name) {
      return &contig;
    }
  }
  return tensorflow::errors::NotFound(StrCat("Unknown contig ", contig_name));
}

// Note that start and end are 0-based, and end is exclusive. So end
// can go up to the number of bases on contig.
bool GenomeReference::IsValidInterval(const Range& range) const {
  StatusOr<const nucleus::genomics::v1::ContigInfo*> contig_status =
      Contig(range.reference_name());
  if (!contig_status.ok()) return false;
  const int64 n_bases = contig_status.ValueOrDie()->n_bases();
  return range.start() >= 0 && range.start() <= range.end() &&
         range.start() < n_bases && range.end() <= n_bases;
}

int64 GenomeReference::NTotalBasepairs() const {
  const auto& contigs = Contigs();
  return std::accumulate(
      contigs.cbegin(), contigs.cend(), static_cast<int64>(0),
      [](int64 acc, const nucleus::genomics::v1::ContigInfo& contig) {
        return acc + contig.n_bases();
      });
}

}  // namespace nucleus
