#pragma once
#include <CPT/format/fileio.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

std::string complement(const std::string& in)
{
  std::string res;
  std::transform(in.begin(), in.end(),
    std::back_inserter(res), [](const char b)
    {
      return (b == 'A')? 'T': (b == 'T')? 'A':
             (b == 'C')? 'G': (b == 'G')? 'C': b;
    });
  return res;
}

std::string reverse(const std::string& in)
{
  std::string res;
  std::reverse_copy(in.begin(), in.end(), std::back_inserter(res));
  return res;
}

inline std::string print_direction(uint8_t direction)
{
  return (direction == 1)? "Sense":
         (direction == 2)? "Antisense": "Unknown";
}

class CDFCell
{
 public:
  int32_t   atom;
  uint16_t  x;
  uint16_t  y;
  int32_t   id; // augmentation
  int32_t   index;
  char      pbase;
  char      tbase;
  uint16_t  length;
  uint16_t  group;

  class CDFBlock* block; // augmentation
  std::string sequence;

 public:
  CDFCell(void)
    : atom(0), x(0), y(0), id(0), index(0)
    , pbase('N'), tbase('N'), length(0), group(0)
    , block(nullptr), sequence("N/A")
  {}

  void parse(std::istream& is, const unsigned version)
  {
    atom = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    x = cpt::format::FileIO<uint16_t, cpt::format::LittleEndian>::read(is);
    y = cpt::format::FileIO<uint16_t, cpt::format::LittleEndian>::read(is);
    index = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    pbase = std::toupper(cpt::format::FileIO<char, cpt::format::LittleEndian>::read(is));
    tbase = std::toupper(cpt::format::FileIO<char, cpt::format::LittleEndian>::read(is));
    if (version >= 2)
    {
      length = cpt::format::FileIO<uint16_t, cpt::format::LittleEndian>::read(is);
      group  = cpt::format::FileIO<uint16_t, cpt::format::LittleEndian>::read(is);
    }
  }

  void print(std::ostream& os = std::cout) const
  {
    os << "\n    -   atom/index: " << atom << " " << index
       << "\n    -   probeid/xy: " << "(" << x << ", " << y << ") " << id
       << "\n    -  pbase/tbase: " << pbase << "/" << tbase
       << "\n    -     sequence: " << sequence
       << "\n    -       length: " << length
       << "\n    -        group: " << group
                                   << std::endl;
  }
};
class CDFBlock
{
 public:
  int32_t   n_atoms;
  int32_t   n_cells;
  uint8_t   n_cells_per_atom;
  uint8_t   direction;
  int32_t   pos_of_1st_atom;
  char      name[64];
  uint16_t  wobble_situation;
  uint16_t  allele_code;
  uint8_t   channel;
  uint8_t   reptype;

  class CDFUnit* unit; // augmentation
  std::vector<CDFCell> cells;

 public:
  CDFBlock(void) = default;

  void parse(std::istream& is, const unsigned version)
  {
    int32_t unused __attribute__((unused));
    n_atoms = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    n_cells = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    n_cells_per_atom = cpt::format::FileIO<uint8_t, cpt::format::LittleEndian>::read(is);
    direction = cpt::format::FileIO<uint8_t, cpt::format::LittleEndian>::read(is);
    pos_of_1st_atom = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    unused = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    is.read(name, sizeof(name));
    if (version >= 2)
    {
      wobble_situation = cpt::format::FileIO<uint16_t, cpt::format::LittleEndian>::read(is);
      allele_code = cpt::format::FileIO<uint16_t, cpt::format::LittleEndian>::read(is);
    }
    if (version >= 3)
    {
      channel = cpt::format::FileIO<uint8_t, cpt::format::LittleEndian>::read(is);
      reptype = cpt::format::FileIO<uint8_t, cpt::format::LittleEndian>::read(is);
    }
    cells.resize(n_cells);
  }

  void print(std::ostream& os = std::cout) const
  {
    os << "\n  +      n_atoms: " << n_atoms
       << "\n  +      n_cells: " << n_cells
       << "\n  + n_cells/atom: " << static_cast<uint16_t>(n_cells_per_atom)
       << "\n  +    direction: " << print_direction(direction)
       << "\n  + 1st atom pos: " << pos_of_1st_atom
       << "\n  +   block name: " << name
       << "\n  +  wobble situ: " << wobble_situation
       << "\n  +  allele code: " << allele_code
       << "\n  +      channel: " << static_cast<uint16_t>(channel)
       << "\n  +      reptype: " << static_cast<uint16_t>(reptype)
                                 << std::endl;
  }
};
class CDFUnit
{
 public:
  char      name[64];
  uint16_t  type;
  uint8_t   direction;
  int32_t   n_atoms;
  int32_t   n_blocks;
  int32_t   n_cells;
  int32_t   number;
  uint8_t   n_cells_per_atom;

  std::vector<CDFBlock> blocks;

 public:
  void parse(std::istream& is, const unsigned version)
  {
    type = cpt::format::FileIO<uint16_t, cpt::format::LittleEndian>::read(is);
    direction = cpt::format::FileIO<uint8_t, cpt::format::LittleEndian>::read(is);
    n_atoms = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    n_blocks = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    n_cells = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    number = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    n_cells_per_atom = cpt::format::FileIO<uint8_t, cpt::format::LittleEndian>::read(is);
    blocks.resize(n_blocks);
  }

  void print(std::ostream& os = std::cout) const
  {
    os << "\n       number: " << number
       << "\n         name: " << name
       << "\n         type: " << type
       << "\n    direction: " << print_direction(direction)
       << "\n      n_atoms: " << n_atoms
       << "\n     n_blocks: " << n_blocks
       << "\n      n_cells: " << n_cells
       << "\n n_cells/atom: " << static_cast<uint16_t>(n_cells_per_atom)
                              << std::endl;
  }
};

class CDFFile
{
  using VecString = std::vector<std::string>;
  using VecPosition = std::vector<int32_t>;

 public:
  int32_t      magic;
  int32_t      version;
  std::string  guid;
  std::string  md5;
  VecString    probe_array_types;
  uint16_t     num_cols;
  uint16_t     num_rows;
  int32_t      num_units;
  int32_t      num_qcunits;
  std::string  custom_refseq;
  VecPosition  fpos;
  VecPosition  qcfpos;

  std::vector<CDFUnit> units;
  std::vector<CDFCell*> cells; // augmentation

 public:
  void open(std::istream& is)
  {
    // read magic number
    magic = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    if (magic != 67)
    {
      std::ostringstream os;
      os << "Invalid file type (magic number = " << magic << ")";
      std::invalid_argument(os.str());
    }
    
    // read version
    version = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);
    if (!(version >= 1 or version <= 4))
    {
      std::ostringstream os;
      os << "Unsupported version of CDF (version = " << version << ")";
      std::invalid_argument(os.str());
    }

    if (version >= 4)
    {
      // read guid
      guid.resize(cpt::format::FileIO<uint32_t, cpt::format::LittleEndian>::read(is), 0);
      is.read(const_cast<char*>(guid.data()), guid.length());

      // read md5
      md5.resize(32, 0);
      is.read(const_cast<char*>(md5.data()), md5.length());

      // read probe array types
      probe_array_types.resize(cpt::format::FileIO<uint8_t, cpt::format::LittleEndian>::read(is), "");
      for (auto& type: probe_array_types)
      {
        type.resize(cpt::format::FileIO<uint32_t, cpt::format::LittleEndian>::read(is), 0);
        is.read(const_cast<char*>(type.data()), type.length());
      }
    }

    // read the number of columns
    num_cols = cpt::format::FileIO<uint16_t, cpt::format::LittleEndian>::read(is);

    // read the number of rows
    num_rows = cpt::format::FileIO<uint16_t, cpt::format::LittleEndian>::read(is);

    // read the number of probesets
    num_units = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);

    // cpt::format::FileIO the number of QC probesets
    num_qcunits = cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is);

    // read custom reference sequence
    custom_refseq.resize(cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is), 0);
    is.read(const_cast<char*>(custom_refseq.data()), custom_refseq.length());

    // preallocate units and probes
    units.resize(num_units);
    cells.resize(num_cols * num_rows, nullptr);

    // read probeset names
    char name[64];
    for (auto& unit: units)
    {
      is.read(name, 64);
      std::copy_n(name, 64, unit.name);
    }

    // read a list of file positions of QC probes
    for (int i = 0; i != num_qcunits; ++i)
      qcfpos.emplace_back(cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is));

    // read a list of file positions of probes
    for (int i = 0; i != num_units; ++i)
      fpos.emplace_back(cpt::format::FileIO<int32_t, cpt::format::LittleEndian>::read(is));

    // read probesets
    auto fp = fpos.cbegin();
    for (auto& unit: this->units)
    {
      is.seekg(*fp++, std::ios::beg);
      unit.parse(is, version);
      for (auto& block: unit.blocks)
      {
        block.parse(is, version);
        block.unit = &unit;
        for (auto& cell: block.cells)
        {
          cell.parse(is, version);
          cell.id = cell.y * num_cols + cell.x;
          cell.block = &block;
          cells[cell.id] = &cell;
        }
      }
    }
  }

  void print(std::ostream& os = std::cout) const
  {
    os <<   "    magic: " << magic
       << "\n  version: " << version
       << "\n     guid: " << guid
       << "\n      md5: " << md5
       << "\n   n_cols: " << num_cols
       << "\n   n_rows: " << num_rows
       << "\n  n_units: " << num_units
       << "\nn_qcunits: " << num_qcunits
       << "\n   refseq: " << custom_refseq
                          << std::endl;

    for (auto& unit: this->units)
    {
      unit.print(os);
      for (auto& block: unit.blocks)
      {
        block.print(os);
        for (auto& cell: block.cells)
        {
          cell.print(os);
        }
      }
    }
  }
};
