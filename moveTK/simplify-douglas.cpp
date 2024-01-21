#include <tuple>
#include <movetk/io/csv/csv.h>
#include <movetk/io/ProbeTraits.h>
#include <movetk/io/ProbeReader.h>
#include <movetk/ds/ColumnarTrajectory.h>
#include <movetk/ds/TabularTrajectory.h>
#include <movetk/io/TrajectoryReader.h>
#include <movetk/io/ParseDate.h>
#include "movetk/Simplification.h"
#include "movetk/utils/GeometryBackendTraits.h"
#include "movetk/utils/Iterators.h"
class MyDateTime : public movetk::io::ParseDate{
public:
    MyDateTime(std::time_t timestamp=0): ParseDate(timestamp,"%Y-%m-%d %H:%M:%S"){}
};
// Define your probe type
// Define a description for your columns (for your own convenience)
enum ProbeColumns{
    id,
    Timestamp,
    Latitude,
    Longitude
};
using ProbePoint = std::tuple<int, unsigned long int, float, float>;
using DataPoint = std::tuple<int, unsigned long int, float, float>;
using CsvReader = movetk::io::csv::csv< DataPoint,
                                            id,
    Timestamp,
    Latitude,
    Longitude>;

// Does our file have a header
static constexpr bool has_header = true;
// The delimiter in the file that separates fields
static constexpr char delimiter = ',';
// Define the traits for the reader
using ProbeTraits = movetk::io::_ProbeTraits<ProbeColumns,CsvReader,ProbePoint,has_header,delimiter>;
//ProbeParseDate (std::time_t ts=0, std::string date_format="%Y-%m-%d %H:%M:%S")
using trajectory_type = movetk::utils::transfer_types<ProbePoint,  movetk::ds::TabularTrajectory>;

//using tabular_trajectory_type = movetk::ds::TabularTrajectory<double, double, double, std::time_t, std::string, std::string>;
int main(int argc, char** argv){
    // Opening  file
    std::string file_name(argv[1]);
    std::unique_ptr<movetk::io::ProbeReader<ProbeTraits>> probe_reader;
    probe_reader = movetk::io::ProbeReaderFactory::create<ProbeTraits>(file_name);

    using Trajectory = movetk::ds::TabularTrajectoryForProbeType<ProbePoint>;
    static constexpr int SplitByFieldIdx = ProbeColumns::id;
    static constexpr unsigned long int SortByTimestamp = ProbeColumns::Timestamp;
    using TrajectoryTraits = movetk::io::_TrajectoryTraits<ProbeTraits, SplitByFieldIdx, SortByTimestamp, Trajectory>;
    using ProbeInputIterator = typename ProbeTraits::ProbeInputIterator;
    movetk::io::TrajectoryReader<TrajectoryTraits, ProbeInputIterator,true,false> trajectory_reader(probe_reader->begin(), probe_reader->end());

    // siplification procedure

    std::cout << "id,longitude,latitude" << std::endl;

    int n = 0, tt=0;
    for (const auto& trajectory : trajectory_reader){
        tt++;
        using MovetkGeometryKernel = typename GeometryKernel::MovetkGeometryKernel;
        movetk::geom::MakePoint<MovetkGeometryKernel> make_point;
        using PolyLine = std::vector<MovetkGeometryKernel::MovetkPoint>;

        std::vector<PolyLine::const_iterator> result;
        using FindFarthest = movetk::simplification::FindFarthest<MovetkGeometryKernel, GeometryKernel::Norm>;
        float tolerance = std::stof(argv[2]);
        movetk::simplification::DouglasPeucker<MovetkGeometryKernel, FindFarthest> DouglasPeucker(tolerance); // 10 meters 91391



        std::vector<int> ids = trajectory.get<id>();
        std::vector<float> lat = trajectory.get<Latitude>();
        std::vector<unsigned long int> dt = trajectory.get<Timestamp>();
        std::vector<float> lon = trajectory.get<Longitude>();
        PolyLine polyline;
        for (int i = 0; i < trajectory.size(); i++)
        {
            polyline.push_back(make_point({lon[i],lat[i]}));
        }



        // std::cout << "Original polyline has: " << polyline.size() << " vertices\n";
        // std::cout << "{";
        // for (auto it = std::begin(polyline); it!=std::end(polyline); ++it){
        //     std::cout << *it;
        //     std::cout << ";";
        // }
        // std::cout << "}\n";

        DouglasPeucker(std::begin(polyline), polyline.end(), std::back_inserter(result));
        // std::reverse(std::begin(result), std::end(result));
        

        // std::cout << "Simplified polyline has: " << result.size() << " vertices\n";
        n+=result.size();
        // std::cout << "Simplified Polyline: ";
        // std::cout << "{";
        for (auto reference : result) {

            std::cout << ids[0] << ",";
            std::cout << *reference << std::endl;
            // std::cout << ";";
        }
        // std::cout << "}\n";
    }
    std::cerr << "we have " << tt << " trajectories\n";
    std::cerr << "Number of points after simplification " << n << "\n";

    return 0;
}