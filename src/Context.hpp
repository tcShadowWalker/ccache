// Copyright (C) 2020-2022 Joel Rosdahl and other contributors
//
// See doc/AUTHORS.adoc for a complete list of contributors.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#pragma once

#include "Args.hpp"
#include "ArgsInfo.hpp"
#include "Config.hpp"
#include "Digest.hpp"
#include "File.hpp"
#include "MiniTrace.hpp"
#include "NonCopyable.hpp"

#ifdef INODE_CACHE_SUPPORTED
#  include "InodeCache.hpp"
#endif

#include <storage/Storage.hpp>

#include <ctime>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

class SignalHandler;

class Context : NonCopyable
{
public:
  Context();
  ~Context();

  // Read configuration, initialize logging, etc. Typically not called from unit
  // tests.
  void initialize();

  ArgsInfo args_info;
  Config config;

  // Current working directory as returned by getcwd(3).
  std::string actual_cwd;

  // Current working directory according to $PWD (falling back to getcwd(3)).
  std::string apparent_cwd;

  // The original argument list.
  Args orig_args;

  // Time of ccache invocation.
  timeval time_of_invocation;

  // Time of compilation. Used to see if include files have changed after
  // compilation.
  time_t time_of_compilation = 0;

  // Files included by the preprocessor and their hashes.
  std::unordered_map<std::string, Digest> included_files;

  // Uses absolute path for some include files.
  bool has_absolute_include_headers = false;

  // Have we tried and failed to get colored diagnostics?
  bool diagnostics_color_failed = false;

  // The name of the temporary preprocessed file.
  std::string i_tmpfile;

  // The name of the cpp stderr file.
  std::string cpp_stderr;

  // Headers (or directories with headers) to ignore in manifest mode.
  std::vector<std::string> ignore_header_paths;

  // Storage (fronting primary and secondary storage backends).
  storage::Storage storage;

#ifdef INODE_CACHE_SUPPORTED
  // InodeCache that caches source file hashes when enabled.
  mutable InodeCache inode_cache;
#endif

  // PID of currently executing compiler that we have started, if any. 0 means
  // no ongoing compilation.
  pid_t compiler_pid = 0;

  // Files used by the hash debugging functionality.
  std::vector<File> hash_debug_files;

  // Options to ignore for the hash.
  const std::vector<std::string>& ignore_options() const;
  void set_ignore_options(const std::vector<std::string>& options);

  // Original umask before applying the `umask`/`CCACHE_UMASK` configuration, or
  // `nullopt` if there is no such configuration.
  std::optional<mode_t> original_umask;

#ifdef MTR_ENABLED
  // Internal tracing.
  std::unique_ptr<MiniTrace> mini_trace;
#endif

  // Register a temporary file to remove at program exit.
  void register_pending_tmp_file(const std::string& path);

private:
  // Options to ignore for the hash.
  std::vector<std::string> m_ignore_options;

  // [Start of variables touched by the signal handler]

  // Temporary files to remove at program exit.
  std::vector<std::string> m_pending_tmp_files;

  // [End of variables touched by the signal handler]

  friend SignalHandler;
  void unlink_pending_tmp_files();
  void unlink_pending_tmp_files_signal_safe(); // called from signal handler
};

inline const std::vector<std::string>&
Context::ignore_options() const
{
  return m_ignore_options;
}
