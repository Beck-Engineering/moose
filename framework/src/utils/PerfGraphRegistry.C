//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphRegistry.h"

#include "DataIO.h"

namespace moose
{
namespace internal
{

PerfGraphRegistry &
getPerfGraphRegistry()
{
  // In C++11 this is even thread safe! (Lookup "Static Initializers")
  static PerfGraphRegistry perf_graph_registry_singleton;

  return perf_graph_registry_singleton;
}

PerfGraphRegistry::PerfGraphRegistry()
  : GeneralRegistry<std::string, PerfGraphSectionInfo>("PerfGraphRegistry")
{
}

unsigned int
PerfGraphRegistry::registerSection(const std::string & section_name, unsigned int level)
{
  return actuallyRegisterSection(section_name, level, "", false);
}

PerfID
PerfGraphRegistry::registerSection(const std::string & section_name,
                                   unsigned int level,
                                   const std::string & live_message,
                                   const bool print_dots)
{
  if (section_name == "")
    mooseError("Section name not provided when registering timed section!");

  if (live_message == "")
    mooseError("Live message not provided when registering timed section!");

  return actuallyRegisterSection(section_name, level, live_message, print_dots);
}

PerfID
PerfGraphRegistry::actuallyRegisterSection(const std::string & section_name,
                                           unsigned int level,
                                           const std::string & live_message,
                                           const bool print_dots)
{
  const auto create_item = [&section_name, &level, &live_message, &print_dots](const std::size_t id)
  { return PerfGraphSectionInfo(id, section_name, level, live_message, print_dots); };
  return registerItem(section_name, create_item);
}

}
}

void
dataStore(std::ostream & stream, moose::internal::PerfGraphSectionInfo & info, void * context)
{
  dataStore(stream, info._id, context);
  dataStore(stream, info._name, context);
  dataStore(stream, info._level, context);
  dataStore(stream, info._live_message, context);
  dataStore(stream, info._print_dots, context);
}

void
dataLoad(std::istream & stream, moose::internal::PerfGraphSectionInfo & info, void * context)
{
  dataLoad(stream, info._id, context);
  dataLoad(stream, info._name, context);
  dataLoad(stream, info._level, context);
  dataLoad(stream, info._live_message, context);
  dataLoad(stream, info._print_dots, context);
}
