//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ExodusFormatter.h"
#include "Parser.h"
#include "MooseApp.h"
#include "SystemInfo.h"
#include "CommandLine.h"
#include "ActionWarehouse.h"

#include "libmesh/exodusII.h"

// C++
#include <sstream>
#include <vector>

ExodusFormatter::ExodusFormatter() : InputFileFormatter(false) {}

void
ExodusFormatter::printInputFile(ActionWarehouse & wh)
{
  _ss << "####################\n"
      << "# Created by MOOSE #\n"
      << "####################\n";

  // Grab the command line arguments first
  _ss << "### Command Line Arguments ###\n";
  if (wh.mooseApp().commandLine())
  {
    for (const auto & arg : wh.mooseApp().commandLine()->getArguments())
      _ss << " " << arg;
  }
  if (wh.mooseApp().getSystemInfo() != NULL)
  {
    _ss << "### Version Info ###\n"
        << std::setw(25) << "App Version: " << wh.mooseApp().getVersion() << "\n"
        << wh.mooseApp().getSystemInfo()->getInfo() << "\n";
  }

  _ss << std::left << "### Parallelism ###\n"
      << std::setw(25) << "Num Processors: " << wh.mooseApp().n_processors() << "\n"
      << std::setw(25) << "Num Threads: " << libMesh::n_threads() << "\n";

  _ss << "### Input File ###" << std::endl;
  wh.printInputFile(_ss);
}

void
ExodusFormatter::format()
{
  std::string s;
  _input_file_record.clear();

  while (std::getline(_ss, s))
  {
    // MAX_LINE_LENGTH is from ExodusII
    if (s.length() > MAX_LINE_LENGTH)
    {
      const std::string continuation("...");
      const size_t cont_len = continuation.length();
      size_t num_lines = s.length() / (MAX_LINE_LENGTH - cont_len) + 1;
      std::string split_line;
      for (size_t j = 0, l_begin = 0; j < num_lines; ++j, l_begin += MAX_LINE_LENGTH - cont_len)
      {
        size_t l_len = MAX_LINE_LENGTH - cont_len;
        if (s.length() < l_begin + l_len)
          l_len = s.length() - l_begin;

        split_line = s.substr(l_begin, l_len);

        if (l_begin + l_len != s.length())
          split_line += continuation;

        _input_file_record.push_back(split_line);
      }
    }
    else
      _input_file_record.push_back(s);
  }
}
