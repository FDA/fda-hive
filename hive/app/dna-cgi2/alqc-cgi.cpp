/*
 *  ::718604!
 * 
 * Copyright(C) November 20, 2014 U.S. Food and Drug Administration
 * Authors: Dr. Vahan Simonyan (1), Dr. Raja Mazumder (2), et al
 * Affiliation: Food and Drug Administration (1), George Washington University (2)
 * 
 * All rights Reserved.
 * 
 * The MIT License (MIT)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <map>
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>
#include <limits>
#include <algorithm>
#include <cctype>

#include <ion/sJson.hpp>
#include <qlib/QPrideCGI.hpp>
using namespace slib;

namespace {

    const char* const MISSING_VALUE = "missing";

    enum enumAlqcCommands
    {
        eAlqcGetTsv
    };

    enum { ASSEMBLY_QC, NGS_QC, BIOSAMPLE_META, INPUT_COUNT };

    struct CaseInsensitiveCompare
    {
        bool operator()(const std::string& a, const std::string& b) const
        {
            return std::lexicographical_compare(
                a.begin(), a.end(),
                b.begin(), b.end(),
                [](unsigned char ac, unsigned char bc)
                {
                    return std::tolower(ac) < std::tolower(bc);
                }
            );
        }
    };

    std::map<std::string, int, CaseInsensitiveCompare> indexMatrix__ = {
        {"argos_Assm_Metrics", ASSEMBLY_QC},
        {"argos_Reads_Metrics", NGS_QC},
        {"argos_ncbi_Metadata", BIOSAMPLE_META}
    };

    int getInputIndex(const char* typeName)
    {
        auto it = indexMatrix__.find(typeName);
        return it != indexMatrix__.end() ? it->second : -1;
    }

    void copyValue(const char* name, sJson& src, sFil& dst, const char* sep = "\t")
    {
        auto val = src.value(name);
        dst.printf("%s%s", val ? val : MISSING_VALUE, sep);
    }

    void copyValue(const char* name, JSNode& src, sFil& dst, const char* sep = "\t")
    {
        if(!src.ok())
        {
            dst.printf("%s%s", MISSING_VALUE, sep);
            return;
        }

        auto val = src[name];
        dst.printf("%s%s", val.ok() ? val.operator const char*() : MISSING_VALUE, sep);
    }

    void copyNcbiMetaValue(const char* name, std::vector<std::unique_ptr<sJson>>& src, sFil& dst, const char* sep = "\t")
    {
        for(auto& ptr : src)
        {
            auto val = ptr->value(name);
            if(val)
            {
                dst.printf("%s%s", val, sep);
                return;
            }
        }

        dst.printf("%s%s", MISSING_VALUE, sep);
    }

    void fillAssemblyQcHeader(sFil& tbl)
    {
        tbl.printf( "organism_name\t"
                    "infraspecific_name\t"
                    "assembled_genome_acc\t"
                    "genome_assembly_id\t"
                    "representative_genome_acc\t"
                    "representative_genome_org\t"
                    "representative_genome_uniprot_acc\t"
                    "lineage\t"
                    "taxonomy_id\t"
                    "bco_id\t"
                    "schema_version\t"
                    "analysis_platform\t"
                    "analysis_platform_object_id\t"
                    "assembly_file_source\t"
                    "genomic_section\t"
                    "num_chromosomes\t"
                    "num_genes\t"
                    "assembly_gc_content\t"
                    "length\t"
                    "size_gaps\t"
                    "size_contigs\t"
                    "contig_percentile\t"
                    "contig_momentum\t"
                    "coverage_contigs\t"
                    "cnt_contigs\t"
                    "coverage_gaps\t"
                    "cnt_gaps\t"
                    "gap_percentile\t"
                    "genome_coverage\t"
                    "n50\t"
                    "n75\t"
                    "n90\t"
                    "n95\t"
                    "l50\t"
                    "l75\t"
                    "l90\t"
                    "l95\t"
                    "phred_average\t"
                    "count_major_mutations\t"
                    "count_major_indels\t"
                    "mutation_momentum\t"
                    "indels_momentum\t"
                    "major_mutation_momentum\t"
                    "major_indels_momentum\t"
                    "alignment_anisotropy\t"
                    "overhang_momentum\t"
                    "aligned_momentum\t"
                    "entropic_momentum\t"
                    "reads_unaligned\t"
                    "percent_reads_unaligned\t"
                    "percent_reads_aligned\t"
                    "reads_aligned\t"
                    "assembly_level\t"
                    "rpkm\r\n");
    }

    bool fillAssemblyQC(sJson& input, std::vector<std::unique_ptr<sJson>>& ncbiMeta, sFil& tbl)
    {
        JSNode root(&input,"$root");
        JSNode& refseq = root["refseq"];
        if(!refseq.ok() || !refseq.dim())
        {
            return false;
        }

        for(idx i = 0; i < refseq.dim(); ++i)
        {
            JSNode& seq = refseq[i];
            copyNcbiMetaValue("organism_name", ncbiMeta, tbl);
            copyNcbiMetaValue("strain", ncbiMeta, tbl);
            copyValue("assembled_genome_acc", seq, tbl);
            copyNcbiMetaValue("accession", ncbiMeta, tbl);
            copyNcbiMetaValue("representative_genome_acc", ncbiMeta, tbl);
            copyNcbiMetaValue("representative_genome_org", ncbiMeta, tbl);
            copyNcbiMetaValue("representative_genome_uniprot_acc", ncbiMeta, tbl);
            copyNcbiMetaValue("lineage", ncbiMeta, tbl);
            copyNcbiMetaValue("taxonomy_id", ncbiMeta, tbl);
            copyNcbiMetaValue("bco_id", ncbiMeta, tbl);
            copyNcbiMetaValue("schema_version", ncbiMeta, tbl);
            copyValue("analysis_platform", seq, tbl);
            copyValue("analysis_platform_object_id", seq, tbl);
            copyNcbiMetaValue("annotation_provider", ncbiMeta, tbl);
            copyNcbiMetaValue("assembly_level", ncbiMeta, tbl);
            copyNcbiMetaValue("num_chromosomes", ncbiMeta, tbl);
            copyNcbiMetaValue("num_genes", ncbiMeta, tbl);
            copyValue("assembly_gc_content", seq, tbl);
            copyValue("length", seq, tbl);
            copyValue("size_gaps", seq, tbl);
            copyValue("size_contigs", seq, tbl);
            copyValue("contig_percentile", seq, tbl);
            copyValue("contig_momentum", seq, tbl);
            copyValue("coverage_contigs", seq, tbl);
            copyValue("cnt_contigs", seq, tbl);
            copyValue("coverage_gaps", seq, tbl);
            copyValue("cnt_gaps", seq, tbl);
            copyValue("gap_percentile", seq, tbl);
            copyValue("genome_coverage", seq, tbl);
            copyValue("n50", seq, tbl);
            copyValue("n75", seq, tbl);
            copyValue("n90", seq, tbl);
            copyValue("n95", seq, tbl);
            copyValue("l50", seq, tbl);
            copyValue("l75", seq, tbl);
            copyValue("l90", seq, tbl);
            copyValue("l95", seq, tbl);
            copyValue("phred_average", seq, tbl);
            copyValue("count_major_mutations", seq, tbl);
            copyValue("count_major_indels", seq, tbl);
            copyValue("mutation_momentum", seq, tbl);
            copyValue("indels_momentum", seq, tbl);
            copyValue("major_mutation_momentum", seq, tbl);
            copyValue("major_indels_momentum", seq, tbl);
            copyValue("alignment_anisotropy", seq, tbl);
            copyValue("overhang_momentum", seq, tbl);
            copyValue("aligned_momentum", seq, tbl);
            copyValue("entropic_momentum", seq, tbl);
            copyValue("reads_unaligned", seq, tbl);
            copyValue("percent_reads_unaligned", seq, tbl);
            copyValue("percent_reads_aligned", seq, tbl);
            copyValue("reads_aligned", seq, tbl);
            copyNcbiMetaValue("assembly_level", ncbiMeta, tbl);
            copyValue("rpkm", seq, tbl, "\r\n");
        }

        return true;
    }

    void fillNgsQcHeader(sFil& tbl)
    {
        tbl.printf( "organism_name\t"
                    "infraspecific_name\t"
                    "genome_assembly_id\t"
                    "representative_genome_acc\t"
                    "representative_genome_org\t"
                    "representative_genome_uniprot_acc\t"
                    "lineage\t"
                    "taxonomy_id\t"
                    "bco_id\t"
                    "schema_version\t"
                    "analysis_platform\t"
                    "analysis_platform_object_id\t"
                    "strain\t"
                    "bioproject\t"
                    "biosample\t"
                    "sra_run_id\t"
                    "ngs_read_file_name\t"
                    "ngs_read_file_source\t"
                    "ngs_gc_content\t"
                    "avg_phred\t"
                    "min_read_length\t"
                    "num_reads\t"
                    "num_reads_unique\t"
                    "avg_read_length\t"
                    "max_read_length\t"
                    "max_duplicate_read\t"
                    "strategy\t"
                    "instrument\t"
                    "id_method\t"
                    "coding_system\t"
                    "percent_coding\t"
                    "percent_non_coding\t"
                    "complexity_percent\t"
                    "non_complexity_percent\t"
                    "stdev_quality\t"
                    "avg_quality_a\t"
                    "avg_quality_t\t"
                    "avg_quality_g\t"
                    "avg_quality_c\t"
                    "avg_quality_n\t"
                    "count_a\t"
                    "count_t\t"
                    "count_g\t"
                    "count_c\t"
                    "count_n\t"
                    "percent_a\t"
                    "percent_t\t"
                    "percent_g\t"
                    "percent_c\t"
                    "percent_n\t"
                    "count_all_WN\t"
                    "count_all\r\n");
    }

    bool fillNgsQC(sJson& input, std::vector<std::unique_ptr<sJson>>& ncbiMeta, sFil& tbl)
    {
        JSNode root(&input,"$root");
        JSNode& ngsqc = root["ngsqc"];
        if(!ngsqc.ok() || !ngsqc.dim())
        {
            return false;
        }

        for(idx i = 0; i < ngsqc.dim(); ++i)
        {
            JSNode& qc = ngsqc[i];
            copyNcbiMetaValue("organism_name", ncbiMeta, tbl);
            copyNcbiMetaValue("strain", ncbiMeta, tbl);
            copyNcbiMetaValue("accession", ncbiMeta, tbl);
            copyNcbiMetaValue("representative_genome_acc", ncbiMeta, tbl);
            copyNcbiMetaValue("representative_genome_org", ncbiMeta, tbl);
            copyNcbiMetaValue("representative_genome_uniprot_acc", ncbiMeta, tbl);
            copyNcbiMetaValue("lineage", ncbiMeta, tbl);
            copyNcbiMetaValue("taxonomy_id", ncbiMeta, tbl);
            copyNcbiMetaValue("bco_id", ncbiMeta, tbl);
            copyNcbiMetaValue("schema_version", ncbiMeta, tbl);
            copyValue("analysis_platform", qc, tbl);
            copyValue("analysis_platform_object_id", qc, tbl);
            copyNcbiMetaValue("strain", ncbiMeta, tbl);
            copyNcbiMetaValue("bioproject", ncbiMeta, tbl);
            copyNcbiMetaValue("biosample", ncbiMeta, tbl);
            copyNcbiMetaValue("reads", ncbiMeta, tbl);
            copyValue("assembled_genome_acc", qc, tbl);
            tbl.printf("SRR\t");
            copyValue("ngs_gc_content", qc, tbl);
            copyValue("avg_phred", qc, tbl);
            copyValue("min_read_length", qc, tbl);
            copyValue("num_reads", qc, tbl);
            copyValue("num_reads_unique", qc, tbl);
            copyValue("avg_read_length", qc, tbl);
            copyValue("max_read_length", qc, tbl);
            copyValue("max_duplicate_read", qc, tbl);
            copyNcbiMetaValue("assembly_method", ncbiMeta, tbl);
            copyNcbiMetaValue("sequencing_tech", ncbiMeta, tbl);
            copyNcbiMetaValue("id_method", ncbiMeta, tbl);
            copyValue("coding_system", qc, tbl);
            copyValue("percent_coding", qc, tbl);
            copyValue("percent_non_coding", qc, tbl);
            copyValue("complexity_percent", qc, tbl);
            copyValue("non_complexity_percent", qc, tbl);
            copyValue("stdev_quality", qc, tbl);
            JSNode& bases = qc["bases"];
            copyValue("avg_quality_a", bases, tbl);
            copyValue("avg_quality_t", bases, tbl);
            copyValue("avg_quality_g", bases, tbl);
            copyValue("avg_quality_c", bases, tbl);
            copyValue("avg_quality_n", bases, tbl);
            copyValue("count_a", bases, tbl);
            copyValue("count_t", bases, tbl);
            copyValue("count_g", bases, tbl);
            copyValue("count_c", bases, tbl);
            copyValue("count_n", bases, tbl);
            copyValue("percent_a", bases, tbl);
            copyValue("percent_t", bases, tbl);
            copyValue("percent_g", bases, tbl);
            copyValue("percent_c", bases, tbl);
            copyValue("percent_n", bases, tbl);
            copyValue("count_all_WN", qc, tbl);
            copyValue("count_all", qc, tbl, "\r\n");
        }

        return true;
    }

    void fillBiosampleMetaHeader(sFil& tbl)
    {
        tbl.printf( "organism_name\t"
                    "infraspecific_name\t"
                    "lineage\t"
                    "taxonomy_id\t"
                    "bco_id\t"
                    "schema_version\t"
                    "bioproject\t"
                    "biosample\t"
                    "strain\t"
                    "sra_run_id\t"
                    "genome_assembly_id\t"
                    "sample_name\t"
                    "instrument\t"
                    "isolate\t"
                    "collected_by\t"
                    "collection_date\t"
                    "geo_loc_name\t"
                    "isolation_source\t"
                    "lat_lon\t"
                    "culture_collection\t"
                    "host\t"
                    "host_age\t"
                    "host_description\t"
                    "host_disease\t"
                    "host_disease_outcome\t"
                    "host_disease_stage\t"
                    "host_health_state\t"
                    "host_sex\t"
                    "id_method\r\n");
    }

    bool fillBiosampleMeta(sJson& input, sFil& tbl)
    {
        copyValue("organism_name", input, tbl);
        copyValue("strain", input, tbl);
        copyValue("lineage", input, tbl);
        copyValue("tax_id", input, tbl);
        copyValue("bco_id", input, tbl);
        copyValue("schema_version", input, tbl);
        copyValue("bioproject", input, tbl);
        copyValue("biosample", input, tbl);
        copyValue("biosample_strain", input, tbl);
        copyValue("reads", input, tbl);
        copyValue("accession", input, tbl);
        copyValue("sample_name", input, tbl);
        copyValue("sequencing_tech", input, tbl);
        copyValue("isolate", input, tbl);
        copyValue("collected_by", input, tbl);
        copyValue("collection_date", input, tbl);
        copyValue("geo_location", input, tbl);
        copyValue("isolation_source", input, tbl);
        copyValue("lat_lon", input, tbl);
        copyValue("culture_collection", input, tbl);
        copyValue("host", input, tbl);
        copyValue("host_age", input, tbl);
        copyValue("host_description", input, tbl);
        copyValue("host_disease", input, tbl);
        copyValue("host_disease_outcome", input, tbl);
        copyValue("host_disease_stage", input, tbl);
        copyValue("host_health_state", input, tbl);
        copyValue("host_sex", input, tbl);
        copyValue("id_method", input, tbl, "\r\n");
        return true;
    }
}

idx DnaCGI::CmdAlqc(idx cmd)
{
    auto printError = [this] (const char* error)
    {
        dataForm.printf("{\"error\":\"%s\"}", error);
        outHtml();
    };

    auto setObjProps = [] (sUsrObj& obj, const char* srcIds, const char* name, idx size)
    {
        obj.propSet("ext", "tsv");
        obj.propSet("orig_name", name);
        obj.propSet("source", "file://");
        obj.propSet("name", name);
        obj.propSetI("size", size);
        const char* version = "1.1";
        sStr baseTag;
        baseTag.printf(0, "alqcGetTsv/%s", srcIds);
        const char* pTag = baseTag.ptr();
        obj.propSet("base_tag", &version, &pTag, 1, true);
    };

    using fillTblHeaderFunc = void(*)(sFil&);
    using fillTblFunc = bool(*)(sJson&, std::vector<std::unique_ptr<sJson>>&, sFil&);

    auto createTableObject = [setObjProps, this] (  const std::unordered_set<udx>& objIds,
                                                    std::vector<std::unique_ptr<sJson>>& ncbiMeta,
                                                    fillTblHeaderFunc headerFunc,
                                                    fillTblFunc tblFunc,
                                                    const char* name)
    {
        sHiveId qcId;
        m_User.objCreate(qcId, "tsv-table");
        sUsrObj qcReport(m_User, qcId);
        sStr qcPath, srcIds, fileName;
        qcReport.addFilePathname(qcPath, true, "_.tsv");
        sFil qcTbl(qcPath);
        headerFunc(qcTbl);
        int rowCount{0};
        for(auto objId : objIds)
        {
            sJson input;
            m_User.objJson(objId, &input);
            if(srcIds.length())
            {
                srcIds.addString(",");
            }

            srcIds.printf("%llu", objId);
            if(tblFunc(input, ncbiMeta, qcTbl))
            {
                ++rowCount;
            }
        }

        if(!rowCount)
        {
            qcReport.purge();
        }
        else
        {
            fileName.printf(0, "%s-%s.tsv", name, srcIds.ptr());
            setObjProps(qcReport, srcIds.ptr(), fileName.ptr(), qcTbl.length());
        }
    };

    switch(cmd)
    {
    case eAlqcGetTsv:
        {
            sVec<sHiveId> objIds;
            idx len;
            const char* ids = pForm->value("ids", 0, &len);
            if(!ids || !ids[0])
            {
                printError("Missing source object Ids");
                break;
            }

            sHiveId::parseRangeSet(objIds, ids, len);
            if(!objIds.dim())
            {
                printError("Missing source object Ids");
                break;
            }


            int index{-1};
            std::unordered_set<udx> inputFiles[INPUT_COUNT];
            for(idx i = 0; i < objIds.dim(); ++i)
            {
                auto objId = objIds[i].objId();
                sUsrObj obj(m_User, sHiveId(objId, 0));
                if(!obj.Id().valid())
                {
                    continue;
                }

                index = getInputIndex(obj.getTypeName());
                if(index == -1)
                {
                    continue;
                }

                inputFiles[index].emplace(objId);
            }

            if(inputFiles[BIOSAMPLE_META].empty())
            {
                printError("At least Biosample Meta object Id must be provided");
                break;
            }

            int rowCount{0};
            sHiveId ncbiId;
            m_User.objCreate(ncbiId, "tsv-table");
            sUsrObj ncbiReport(m_User, ncbiId);
            sStr ncbiPath;
            ncbiReport.addFilePathname(ncbiPath, true, "_.tsv");
            sFil ncbiTbl(ncbiPath);
            fillBiosampleMetaHeader(ncbiTbl);

            sStr srcIds;
            std::vector<std::unique_ptr<sJson>> ncbiMeta;
            for(auto objId : inputFiles[BIOSAMPLE_META])
            {
                if(srcIds.length())
                {
                    srcIds.addString(",");
                }

                srcIds.printf("%llu", objId);
                ncbiMeta.emplace_back(std::make_unique<sJson>());
                const auto& jsonPtr = ncbiMeta.back();
                m_User.objJson(objId, jsonPtr.get());
                if(fillBiosampleMeta(*jsonPtr, ncbiTbl))
                {
                    ++rowCount;
                }
            }

            if(!rowCount)
            {
                ncbiReport.purge();
                printError("No valid Biosample Meta object Id has been provided");
                break;
            }

            sStr fileName;
            fileName.printf(0, "argos_ncbi_Metadata-%s.tsv", srcIds.ptr());
            setObjProps(ncbiReport, srcIds.ptr(), fileName.ptr(), ncbiTbl.length());

            if(inputFiles[ASSEMBLY_QC].size())
            {
                createTableObject(inputFiles[ASSEMBLY_QC], ncbiMeta, fillAssemblyQcHeader, fillAssemblyQC, "argos_Assm_Metrics");
            }

            if(inputFiles[NGS_QC].size())
            {
                createTableObject(inputFiles[NGS_QC], ncbiMeta, fillNgsQcHeader, fillNgsQC, "argos_Reads_Metrics");
            }
        }
        break;
    default:
        break;
    }
 
    return 1;
}
