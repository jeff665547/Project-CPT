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
namespace cpt {
namespace format {
/**
* @brief Convert a DNA sequence into its complement counterpart
* @param in denotes the input DNA sequence
* @return complemented sequence
*/
auto complement(const std::string& in)
{
// FIXME CCD has same library for this function, which is more abstract and flexible 
    std::string res;
    std::transform(
        in.begin()
      , in.end()
      , std::back_inserter(res)
      , [](const char b)
        {
            return (b == 'A')? 'T': (b == 'T')? 'A':
                   (b == 'C')? 'G': (b == 'G')? 'C': b;
        }
    );
    return res;
}

/**
* @brief Convert a DNA sequence into its reverse counterpart
* @param in denotes the input DNA sequence.
* @return reversed sequence.
*/
auto reverse(const std::string& in)
{
// FIXME CCD has same library for this function, which is more abstract and flexible 
    std::string res;
    std::reverse_copy(
        in.begin()
      , in.end()
      , std::back_inserter(res)
    );
    return res;
}

/**
* @brief Decoding the strand of designed sequence
* @param direction is a number denoting the strand of designed sequence.
* @return Sense, Antisenes or Unknown.
*/
inline std::string print_direction(uint8_t direction)
{
    return (direction == 1)? "Sense":
           (direction == 2)? "Antisense":
                             "Unknown";
}
// FIXME class member indent

/**
* @brief CDFCell defines a data structure for probe information.
*        It is a part of XDA format in Affymetrix.
*/
class CDFCell
{
  public:
    int32_t   atom;        //!< An index used to group multiple cells.
    uint16_t  x;           //!< X coordinate of the cell.
    uint16_t  y;           //!< Y coordinate of the cell.
    int32_t   id;          /**< Probe ID. 0-based. id = x + y * num_cols.
                                Notice that this member is not a part of
                                XDA format, but augmented by this project. */
    int32_t   index;       /**< An index used to look up the
                                corresponding cell data in the CEL file.
                                It is relative to sequence for
                                [1] CustomSeq,
                                [2] Genotyping, 
                                [3] Copy Number, 
                                [4] Polymorphic Marker,
                                [5] Multichannel Marker units
                                for Expression units this value
                                is the atom number. */
    char      pbase;       //!< Base of probe at substitution position.
    char      tbase;       //!< Base of probe at interrogation position.
    uint16_t  length;      /**< Length of probe sequence.
                                (only available in ver. 2, 3 and 4) */
    uint16_t  group;       /**< Physical grouping of probe.
                               (only available in ver 2, 3 and 4) */
    std::string sequence;  /**< Probe sequence.
                                Notice that this member is not a part of
                                XDA format, but augmented by this project. */
    class CDFBlock* block; /**< a link to CDFBlock.  It is augmented by
                                this project for convenience. */

  public:
    /**
    * @brief Default constructor
    */
    CDFCell(void) 
      : atom(0)
      , x(0)
      , y(0)
      , id(0)
      , index(0)
      , pbase('N')
      , tbase('N')
      , length(0)
      , group(0)
      , sequence("")
      , block(nullptr)
    {}

    /**
    * @brief Load CDF cell data
    * @param is denotes the input stream.
    * @param version means current schema version.
    */
    void parse(std::istream& is, const unsigned version)
    {
        atom = FileIO<int32_t, LittleEndian>::read(is);
        x = FileIO<uint16_t, LittleEndian>::read(is);
        y = FileIO<uint16_t, LittleEndian>::read(is);
        index = FileIO<int32_t, LittleEndian>::read(is);
        pbase = std::toupper(FileIO<char, LittleEndian>::read(is));
        tbase = std::toupper(FileIO<char, LittleEndian>::read(is));
        if (version >= 2)
        {
            length = FileIO<uint16_t, LittleEndian>::read(is);
            group  = FileIO<uint16_t, LittleEndian>::read(is);
        }
    }

    /**
    * @brief Print data members
    * @param os denotes the output stream. (default: std::cout)
    */
    void print(std::ostream& os = std::cout) const
    {
        os << "\n    - atom/index   " << atom << " " << index
           << "\n    - probe-id/xy  " << id << " (" << x << ", " << y << ")"
           << "\n    - pbase/tbase  " << pbase << "/" << tbase
           << "\n    - sequence     " << sequence
           << "\n    - length       " << length
           << "\n    - group        " << group
           << "\n";
    }
};

/**
* @brief CDFBlock defines a data structure for block information.
*        It is a part of XDA format in Affymetrix.
*/
class CDFBlock
{
  public:
    int32_t  n_atoms;           //!< Number of atoms.
    int32_t  n_cells;           //!< Number of cells.
    uint8_t  n_cells_per_atom;  //!< Number of cells per atom.
    uint8_t  direction;         //!< Strand. (unused in Axiom array)
    int32_t  pos_of_1st_atom;   //!< The position of the first atom.
    char     name[64];          //!< The block name.
    uint16_t wobble_situation;  /**< The wobble situation for
                                     Polymorphic Marker and
                                     Multichannel Marker units in the block.
                                     (only available in ver. 2, 3 and 4) */
    uint16_t allele_code;       /**< The allele code for Polymorphic Marker and
                                     Multichannel Marker units in the block.
                                     (only available in ver. 2, 3 and 4) */
    uint8_t  channel;           /**< The channel code for multichannel
                                     microarray platform.
                                     (only available in ver. 3 and 4) */
    uint8_t  reptype;           /**< The probe replication type:
                                     0 - unknown,
                                     1 - different probe sequences,
                                     2 - some probe sequences are identical,
                                     3 - all probe sequences are identical.
                                     (only avaiblbe in ver. 3 and 4) */
    std::vector<CDFCell> cells; //!< A list of related probes
    class CDFUnit* unit;        /**< a link to CDFUnit. It is augmented by
                                     this project for convenience */

  public:
    /**
    * @brief Default constructor
    */
    CDFBlock(void)
      : n_atoms(0)
      , n_cells(0)
      , n_cells_per_atom(0)
      , direction(0)
      , pos_of_1st_atom(0)
      , name{'\0'}
      , wobble_situation(0)
      , allele_code(0)
      , channel(0)
      , reptype(0)
      , unit(nullptr)
    {}

    /**
    * @brief Load CDF block data
    * @param is denotes the input stream.
    * @param version means current schema version.
    */
    void parse(std::istream& is, const unsigned version)
    {
        int32_t unused __attribute__((unused));
        n_atoms = FileIO<int32_t, LittleEndian>::read(is);
        n_cells = FileIO<int32_t, LittleEndian>::read(is);
        n_cells_per_atom = FileIO<uint8_t, LittleEndian>::read(is);
        direction = FileIO<uint8_t, LittleEndian>::read(is);
        pos_of_1st_atom = FileIO<int32_t, LittleEndian>::read(is);
        unused = FileIO<int32_t, LittleEndian>::read(is);
        is.read(name, sizeof(name));
        if (version >= 2)
        {
          wobble_situation = FileIO<uint16_t, LittleEndian>::read(is);
          allele_code = FileIO<uint16_t, LittleEndian>::read(is);
        }
        if (version >= 3)
        {
          channel = FileIO<uint8_t, LittleEndian>::read(is);
          reptype = FileIO<uint8_t, LittleEndian>::read(is);
        }
        cells.resize(n_cells);
    }

    /**
    * @brief Print data members
    * @param os denotes the output stream. (default: std::cout)
    */
    void print(std::ostream& os = std::cout) const
    {
        os << "\n  + n_atoms       " << n_atoms
           << "\n  + n_cells       " << n_cells
           << "\n  + n_cells/atom  " << static_cast<uint16_t>(n_cells_per_atom)
           << "\n  + direction     " << print_direction(direction)
           << "\n  + 1st atom pos  " << pos_of_1st_atom
           << "\n  + block name    " << name
           << "\n  + wobble        " << wobble_situation
           << "\n  + allele        " << allele_code
           << "\n  + channel       " << static_cast<uint16_t>(channel)
           << "\n  + reptype       " << static_cast<uint16_t>(reptype)
           << "\n";
    }
};

/**
* @brief CDFUnit defines a data structure for probeset information
*        It is a part of XDA format in Affymetrix.
*/
class CDFUnit
{
  public:
    char     name[64];            /**< The name of probeset.
                                       Notice that this member is not a part of
                                       XDA format, but augmented by this project. */
    uint16_t type;                //!< Probeset type
    uint8_t  direction;           //!< Strand. (unused in Axiom array)
    int32_t  n_atoms;             //!< Number of atoms
    int32_t  n_blocks;            //!< Number of blocks
    int32_t  n_cells;             //!< Number of cells
    int32_t  number;              //!< Probeset number
    uint8_t  n_cells_per_atom;    //!< Number of cells per atom
    std::vector<CDFBlock> blocks; //!< a list of related blocks

 public:
    /**
    * @brief Default constructor
    */
    CDFUnit(void)
      : name{'\0'}
      , type(0)
      , direction(0)
      , n_atoms(0)
      , n_blocks(0)
      , n_cells(0)
      , number(0)
      , n_cells_per_atom(0)
    {}

    /**
    * @brief Load CDF probeset data
    * @param is denotes the input stream.
    * @param version means current schema version.
    */
    void parse(std::istream& is, const unsigned version)
    {
        type = FileIO<uint16_t, LittleEndian>::read(is);
        direction = FileIO<uint8_t, LittleEndian>::read(is);
        n_atoms = FileIO<int32_t, LittleEndian>::read(is);
        n_blocks = FileIO<int32_t, LittleEndian>::read(is);
        n_cells = FileIO<int32_t, LittleEndian>::read(is);
        number = FileIO<int32_t, LittleEndian>::read(is);
        n_cells_per_atom = FileIO<uint8_t, LittleEndian>::read(is);
        blocks.resize(n_blocks);
    }

    /**
    * @brief Print data members
    * @param os denotes the output stream. (default: std::cout)
    */
    void print(std::ostream& os = std::cout) const
    {
        os << "\n+ number        " << number
           << "\n+ name          " << name
           << "\n+ type          " << type
           << "\n+ direction     " << print_direction(direction)
           << "\n+ n_atoms       " << n_atoms
           << "\n+ n_blocks      " << n_blocks
           << "\n+ n_cells       " << n_cells
           << "\n+ n_cells/atom  " << static_cast<uint16_t>(n_cells_per_atom)
           << "\n";
    }
};

/**
* @brief Chip Description File (XDA format) defined by Affymetrix
*/
class CDFFile
{
  private:
    using VecString = std::vector<std::string>;
    using VecPosition = std::vector<int32_t>;

  private:
    VecPosition  qcfpos;             //!< File start position for each QC unit
    VecPosition  fpos;               //!< File start position for each unit

  public:
    int32_t      magic;              //!< File signature, always magic = 67
    int32_t      version;            //!< File version = 1, 2, 3 or 4
    std::string  guid;               //!< GUID
    std::string  md5;                //!< MD5
    VecString    probe_array_types;  //!< Array type
    uint16_t     num_cols;           //!< Number of feature columns
    uint16_t     num_rows;           //!< Number of feature rows
    int32_t      num_units;          //!< Number of probesets
    int32_t      num_qcunits;        //!< Number of QC probesets
    std::string  custom_refseq;      //!< CustomSeq reference sequence
    std::vector<CDFUnit> units;      //!< A list of probesets
    std::vector<CDFCell*> cells;     /**< A view to each probe information.
                                          Notice that this view may contain
                                          some pointers = nullptr.
                                          This is not a part of XDA format,
                                          but augmented by this project. */
  public:
    CDFFile(void)
      : magic(67)
      , version(0)
      , guid("")
      , md5("")
      , probe_array_types()
      , num_cols(0)
      , num_rows(0)
      , num_units(0)
      , num_qcunits(0)
      , custom_refseq("")
      , units()
      , cells()
    {}

    /**
    * @brief Load CDF probeset data
    * @param is denotes the input stream.
    * @param version means current schema version.
    */
    void open(std::istream& is)
    {
        // read magic number
        magic = FileIO<int32_t, LittleEndian>::read(is);
        if (magic != 67)
        {
            std::ostringstream os;
            os << "Invalid file type (magic number = " << magic << ")";
            throw std::invalid_argument(os.str());
        }
        
        // read version
        version = FileIO<int32_t, LittleEndian>::read(is);
        if (!(version >= 1 or version <= 4))
        {
            std::ostringstream os;
            os << "Unsupported version of CDF (version = " << version << ")";
            throw std::invalid_argument(os.str());
        }

        if (version >= 4)
        {
            // read guid
            guid.resize(FileIO<uint32_t, LittleEndian>::read(is), 0);
            is.read(const_cast<char*>(guid.data()), guid.length());

            // read md5
            md5.resize(32, 0);
            is.read(const_cast<char*>(md5.data()), md5.length());

            // read probe array types
            probe_array_types.resize(FileIO<uint8_t, LittleEndian>::read(is), "");
            for (auto& type: probe_array_types)
            {
                type.resize(FileIO<uint32_t, LittleEndian>::read(is), 0);
                is.read(const_cast<char*>(type.data()), type.length());
            }
        }

        // read the number of columns
        num_cols = FileIO<uint16_t, LittleEndian>::read(is);

        // read the number of rows
        num_rows = FileIO<uint16_t, LittleEndian>::read(is);

        // read the number of probesets
        num_units = FileIO<int32_t, LittleEndian>::read(is);

        // FileIO the number of QC probesets
        num_qcunits = FileIO<int32_t, LittleEndian>::read(is);

        // read custom reference sequence
        custom_refseq.resize(FileIO<int32_t, LittleEndian>::read(is), 0);
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
            qcfpos.emplace_back(FileIO<int32_t, LittleEndian>::read(is));

        // read a list of file positions of probes
        for (int i = 0; i != num_units; ++i)
            fpos.emplace_back(FileIO<int32_t, LittleEndian>::read(is));

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

    /**
    * @brief Print data members
    * @param os denotes the output stream. (default: std::cout)
    */
    void print(std::ostream& os = std::cout) const
    {
        os <<   "magic      " << magic
           << "\nversion    " << version
           << "\nguid       " << guid
           << "\nmd5        " << md5
           << "\nn_cols     " << num_cols
           << "\nn_rows     " << num_rows
           << "\nn_units    " << num_units
           << "\nn_qcunits  " << num_qcunits
           << "\nrefseq     " << custom_refseq
           << "\n";

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

} // namespace format
} // namespace cpt
