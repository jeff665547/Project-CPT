#pragma once
#include <Nucleona/format/csv_parser.hpp>
namespace cpt::format {
    template<int Pos, class T>
    using PMT               = nucleona::format::PMT<Pos, T>;

    template<template<class... T> class Entry, class... ColPair>
    using TraitPrototype    = nucleona::format::CsvTupleEntryTrait<
        Entry, ColPair...
    >;

    template<class TRAIT>
    using TupleParser       = nucleona::format::CsvParser<TRAIT>;

}