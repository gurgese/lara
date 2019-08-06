// ===========================================================================
//                LaRA: Lagrangian Relaxed structural Alignment
// ===========================================================================
// Copyright (c) 2016-2019, Jörg Winkler, Freie Universität Berlin
// Copyright (c) 2016-2019, Gianvito Urgese, Politecnico di Torino
// Copyright (c) 2006-2019, Knut Reinert, Freie Universität Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// * Neither the name of Jörg Winkler, Gianvito Urgese, Knut Reinert,
//   the FU Berlin or the Politecnico di Torino nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL KNUT REINERT OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.

#pragma once

/*!\file parameters.hpp
 * \brief This file contains all settings and parameters of LaRA.
 */

#include <thread>

#include <seqan/arg_parse.h>
#include <seqan/score.h>

#include "data_types.hpp"
#include "rna_score_matrices.hpp"

namespace lara
{

template<typename TSequenceValue, typename TTag>
inline void setRnaScoreMatrix(seqan::Score<float, seqan::ScoreMatrix<TSequenceValue>> & matrix, TTag)
{
    float const * tab = RnaScoringMatrixData_<float, TSequenceValue, TTag>::getData();
    seqan::arrayCopy(tab, tab + RnaScoringMatrixData_<float, TSequenceValue, TTag>::TAB_SIZE,
                     matrix.data_tab);
}

class Parameters
{
public:
    // Name of input file
    seqan::CharString        inFile{};
    // Name of input fileRef
    seqan::CharString        inFileRef{};
    // Name of dotplot file
    std::vector<std::string> dotplotFile{};
    // Name of output file (default: stdout)
    seqan::CharString        outFile{};
    // number of iterations
    unsigned                 numIterations{500u};
    // number of non-decreasing iterations
    unsigned                 maxNondecrIterations{50u};
    // value to be considered for the equality of upper and lower bounds difference
    float                    epsilon{0.01f};
    // my, necessary for computing appropriate step sizes
    float                    stepSizeFactor{1.0f};
    // scoring matrix name that should be used for scoring alignment edges in the actual problem
    seqan::CharString        laraScoreMatrixName{};
    //    Score<float, ScoreMatrix<Rna5, Default> > laraScoreMatrix;
    RnaScoreMatrix           laraScoreMatrix;
    //    TScoringSchemeRib laraScoreMatrixRib;
    // Gap open and extend costs for generating the alignment edges
    float                    suboptimalDiff{40.0f};
    // Gap open and extend costs for generating the alignment edges
    float                    laraGapOpen{-6.0f};
    float                    laraGapExtend{-2.0f};
    // scaling factor for the scores of the alignment edges
    // Specifies the contribution of the sequence scores (specified by the larascore matrix) to the overall structural
    // alignment.
    float                    sequenceScale{1.0f};
    float                    balance{0.0f};
    // scoring mode, either LOGARITHMIC or SCALE
    unsigned                 structureScoring{ScoringMode::LOGARITHMIC};
    // specify the method to be used to create the T-Coffe library
    std::pair<unsigned, unsigned> libraryScore{};
    bool                     libraryScoreIsLinear{};
    Status                   status;
    unsigned                 matching{5u};
    unsigned                 num_threads{1u};

    Parameters(int argc, char const ** argv)
    {
        status = setParameters(argc, argv);
    }

private:
    inline Status setParameters(int argc, char const ** argv)
    {
        using namespace seqan;
        ArgumentParser parser;

        setAppName(parser, "lara");
        setShortDescription(parser, "Lagrangian Relaxed Alignment for RNA structures");
        setVersion(parser, "2.0.1");
        setDate(parser, "July 2019");
        addDescription(parser, "RNA structural alignment algorithm.");

        addUsageLine(parser, R"( -i \fIinFile\fP [\fIparameters\fP])");
        addUsageLine(parser, R"( -d \fIdpFile\fP -d \fIdpFile\fP [-d ...] [\fIparameters\fP])");

        addOption(parser, ArgParseOption("v", "verbose",
                                         "0: no additional outputs, 1: global statistics, "
                                         "2: extensive statistics for each batch of reads, 3: Debug output. (0)",
                                         ArgParseArgument::INTEGER, "INT"));
        setMinValue(parser, "verbose", "0");
        setMaxValue(parser, "verbose", "3");

        // Input options
        addSection(parser, "Input Options");

        addOption(parser, ArgParseOption("i", "infile",
                                         "Path to the input file",
                                         ArgParseArgument::INPUT_FILE, "IN"));

        addOption(parser, ArgParseOption("r", "reffile",
                                         "Path to the reference input file",
                                         ArgParseArgument::INPUT_FILE, "IN"));

        addOption(parser, ArgParseOption("d", "dotplot",
                                         "Use dotplot files from RNAfold (*_dp.ps) as sequence and structure input.",
                                         ArgParseArgument::INPUT_FILE, "IN", true));

        // Output options
        addSection(parser, "Output Options");

        addOption(parser, ArgParseOption("w", "write",
                                         "Path to the output file (default: stdout)",
                                         ArgParseArgument::OUTPUT_FILE, "OUT"));

        addOption(parser, ArgParseOption("l", "libscore",
                                         "The range of the scores for the T-Coffe library. "
                                         "Default: 500 1000 (binary)",
                                         ArgParseArgument::INTEGER, "MIN MAX",
                                         false, 2));

        // Alignment options
        addSection(parser, "LaRA Alignment Options");

        addOption(parser, ArgParseOption("n", "numiter",
                                         "number of iterations. ",
                                         ArgParseArgument::INTEGER, "INT"));
        setMinValue(parser, "numiter", "1");

        addOption(parser, ArgParseOption("a", "maxnondecreasing",
                                         "number of non-decreasing iterations. (50)",
                                         ArgParseArgument::INTEGER, "INT"));
        setMinValue(parser, "maxnondecreasing", "0");

        addOption(parser, ArgParseOption("e", "epsilon",
                                         "value to be considered for the equality of upper and lower bounds difference",
                                         ArgParseArgument::DOUBLE, "DOUBLE"));

        addOption(parser, ArgParseOption("f", "factor",
                                         "necessary for computing appropriate step sizes.",
                                         ArgParseArgument::DOUBLE, "DOUBLE"));

        addOption(parser, ArgParseOption("s", "scorematrix",
                                         "scoring matrix name that should be used for scoring alignment edges in the "
                                         "actual problem",
                                         ArgParseOption::STRING));

        addOption(parser, ArgParseOption("u", "subopt",
                                         "Parameter for filtering alignment edges. Only those are created, whose prefix"
                                         " score + suffix score in the DP matrix is at most subopt below the "
                                         "optimal score.",
                                         ArgParseArgument::DOUBLE, "DOUBLE"));

        addOption(parser, ArgParseOption("y", "gapopen",
                                         "Gap open costs for generating the alignment edges",
                                         ArgParseArgument::DOUBLE, "DOUBLE"));

        addOption(parser, ArgParseOption("x", "gapextend",
                                         "Gap extend costs for generating the alignment edges",
                                         ArgParseArgument::DOUBLE, "DOUBLE"));

        addOption(parser, ArgParseOption("c", "seqscale",
                                         "Scaling factor for the sequence scores (below 1 gives more impact for "
                                         "structure).",
                                         ArgParseArgument::DOUBLE, "DOUBLE"));

        addOption(parser, ArgParseOption("b", "balance",
                                         "Factor of how much the sequence identity should influence the balance "
                                         "of sequence and structure score. (0)",
                                         ArgParseArgument::DOUBLE, "DOUBLE"));

        addOption(parser, ArgParseOption("p", "probscoremode",
                                         "base pair probability scoring mode, either LOGARITHMIC (0), SCALE (1). (0)",
                                         ArgParseArgument::INTEGER, "INT"));
        setMinValue(parser, "probscoremode", "0");
        setMaxValue(parser, "probscoremode", "1");

        addOption(parser, ArgParseOption("m", "matching",
                                         "Lookahead for greedy matching algorithm. Value 0 uses LEMON instead. (5)",
                                         ArgParseArgument::INTEGER, "INT"));
        setMinValue(parser, "matching", "0");

        addOption(parser, ArgParseOption("j", "threads",
                                         "Use the number of specified threads. Decrease if memory problems occur. "
                                         "Value 0 tries to detect the maximum number. (1)",
                                         ArgParseArgument::INTEGER, "INT"));
        setMinValue(parser, "threads", "0");

        ArgumentParser::ParseResult parseResult = parse(parser, argc, argv);
        if (parseResult != ArgumentParser::PARSE_OK)
            return parseResult == ArgumentParser::ParseResult::PARSE_ERROR ? Status::EXIT_ERROR : Status::EXIT_OK;

        getOptionValue(_VERBOSE_LEVEL, parser, "verbose");
        getOptionValue(numIterations, parser, "numiter");
        getOptionValue(maxNondecrIterations, parser, "maxnondecreasing");
        getOptionValue(epsilon, parser, "epsilon");
        getOptionValue(stepSizeFactor, parser, "factor");
        getOptionValue(laraScoreMatrixName, parser, "scorematrix");
        getOptionValue(suboptimalDiff, parser, "subopt");
        getOptionValue(laraGapOpen, parser, "gapopen");
        getOptionValue(laraGapExtend, parser, "gapextend");
        getOptionValue(sequenceScale, parser, "seqscale");
        getOptionValue(balance, parser, "balance");
        getOptionValue(structureScoring, parser, "probscoremode");
        getOptionValue(libraryScore.first, parser, "libscore", 0);
        getOptionValue(libraryScore.second, parser, "libscore", 1);
        getOptionValue(inFileRef, parser, "reffile");
        getOptionValue(matching, parser, "matching");
        getOptionValue(num_threads, parser, "threads");

        getOptionValue(inFile, parser, "infile");
        unsigned numDotplots = getOptionValueCount(parser, "dotplot");
        dotplotFile.resize(numDotplots);
        for (unsigned idx = 0; idx < numDotplots; ++idx)
        {
            getOptionValue(dotplotFile[idx], parser, "dotplots", idx);
        }

        if (empty(inFile) && dotplotFile.empty())
        {
            printShortHelp(parser);
            return Status::EXIT_ERROR;
        }

        libraryScoreIsLinear = isSet(parser, "l");

        if (num_threads == 0u)
        {
            unsigned nthreads = std::thread::hardware_concurrency();
            if (nthreads != 0)
                num_threads = nthreads;
            else
                num_threads = 1u;
        }

        getOptionValue(outFile, parser, "write");
        if (isSet(parser, "write"))
        {
            _LOG(1, "The specified output file is " << outFile << std::endl);
        }

        // set score matrix
        laraScoreMatrix.data_gap_extend = laraGapExtend;
        laraScoreMatrix.data_gap_open   = laraGapOpen;
        if (empty(laraScoreMatrixName))
        {
            _LOG(2, "Predefined Ribosum65 matrix will be used." << std::endl);
            setRnaScoreMatrix(laraScoreMatrix, Ribosum65N());
        }
        else if (loadScoreMatrix(laraScoreMatrix, toCString(laraScoreMatrixName)))
        {
            _LOG(2, "Provided scoring matrix will be used: " << laraScoreMatrixName << std::endl);
        }
        else
        {
            std::cerr << "Matrix file could not be opened: " << laraScoreMatrixName
                      << "). Predefined Ribosum65 matrix will be used." << std::endl;
            setRnaScoreMatrix(laraScoreMatrix, Ribosum65N());
        }

        return Status::CONTINUE;
    }
};

} // namespace lara
