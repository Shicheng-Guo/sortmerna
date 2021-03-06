#pragma once
/**
* FILE: read.hpp
* Created: Nov 06, 2017 Mon
* Wrapper of a Reads' record and its Match results
*/

#include <string>
#include <sstream>
#include <vector>
#include <algorithm> // std::find_if

#include "kvdb.hpp"
#include "traverse_bursttrie.hpp" // id_win
#include "ssw.hpp" // s_align2
#include "options.hpp"

class References; // forward

struct alignment_struct2
{
	uint32_t max_size; // max size of alignv i.e. max number of alignments to store (see options '-N', '--best N') TODO: remove?
	uint32_t min_index; // index into alignv for the reference with lowest SW alignment score (see 'compute_lis_alignment')
	uint32_t max_index; // index into alignv for the reference with highest SW alignment score (see 'compute_lis_alignment')
	std::vector<s_align2> alignv;

	// constructors
	alignment_struct2();
	alignment_struct2(std::string); // create from binary string

	// member functions
	std::string toString(); // convert to binary string
	size_t getSize();
	void clear();
};

/* 
 * 1. id_win_hits  std::vector<id_win> id_win_hits
 *	  array of positions of window hits on the reference sequence in given index/part.
 *	  Only used during alignment on a particular index/part. No need to store on disk.
 *	  Reset on each new index part
 *    [0] : {id = 568 win = 0 	}
 *	  ...
 *	  [4] : {id = 1248788 win = 72 }
 *	          |            |_k-mer start position on read
 *	          |_k-mer id on reference (index into 'positions_tbl')
 */
class Read 
{
public:
	std::string id; // Read ID: combination of readsfile index and the read index into the readsfile e.g. 0_0, 1_103, etc. Generated upon reading from file.
	std::size_t read_num; // Read number in the reads file starting from 0
	uint8_t readfile_idx; // index into Runopts::readfiles
	bool isValid; // flags the record valid/non-valid
	bool isEmpty; // flags the Read object is empty i.e. just a placeholder for copy assignment
	bool is03; // indicates Read::isequence is in 0..3 alphabet
	bool is04; // indicates Read:iseqeunce is in 0..4 alphabet. Seed search cannot proceed on 0-4 alphabet
	bool isRestored; // flags the read is restored from Database. See 'Read::restoreFromDb'

	std::string header;
	std::string sequence;
	std::string quality; // "" (fasta) | "xxx..." (fastq)
	Format format; // fasta | fastq

	// calculated
	std::string isequence; // sequence in Integer alphabet: [A,C,G,T] -> [0,1,2,3]
	bool reversed; // indicates the read is reverse-complement i.e. 'revIntStr' was applied
	std::vector<int> ambiguous_nt; // positions of ambiguous nucleotides in the sequence (as defined in nt_table/load_index.cpp)

	// store in database ------------>
	unsigned int lastIndex; // last index number this read was aligned against. Set in Processor::callback
	unsigned int lastPart; // last part number this read was aligned against.  Set in Processor::callback
	// matching results
	bool hit; // indicates a match for this Read has been found
	bool hit_denovo; // hit & !(%Cov & %ID) TODO: change this to 'hit_cov_id' because it's set to true if !(%Cov & %ID) regardless of 'hit'
	bool null_align_output; // flags NULL alignment was output to file (needs to be done once only)
	uint16_t max_SW_count; // count of matches that have Max Smith-Waterman score for this read
	int32_t num_alignments; // number of alignments to output per read
	uint32_t readhit; // number of seeds matches between read and database. (? readhit == id_win_hits.size)
	int32_t best; // init with opts.min_lis, see 'this.init'. Don't DB store/restore (bug 51).

	std::vector<id_win> id_win_hits; // [1] positions of hits on the reference sequence in given index/part

	alignment_struct2 hits_align_info; // stored in DB

	std::vector<int8_t> scoring_matrix; // initScoringMatrix   orig: int8_t* scoring_matrix
	// <---- END store in database

public:
	Read();
	Read(std::string id, std::string header, std::string sequence, std::string quality, Format format);
	Read(const Read & that); // copy constructor
	Read & operator=(const Read & that); // copy assignment
	~Read();

public:
	void generate_id();
	void initScoringMatrix(int8_t match, int8_t mismatch, int8_t score_N);
	void validate();
	void clear();
	void init(Runopts & opts);
	std::string matchesToJson();
	void unmarshallJson(KeyValueDatabase & kvdb);
	std::string toString();
	bool load_db(KeyValueDatabase & kvdb);
	void seqToIntStr();
	void revIntStr();
	std::string get04alphaSeq();
	void flip34();
	void calcMismatchGapId(References &refs, int alignIdx, uint32_t &mismatches, uint32_t &gaps, uint32_t &id);
	std::string getSeqId();
	uint32_t hashKmer(uint32_t pos, uint32_t len);
}; // ~class Read
