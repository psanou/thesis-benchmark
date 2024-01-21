#include <tuple>
#include <movetk/io/csv/csv.h>
#include <movetk/io/ProbeTraits.h>
#include <movetk/io/ProbeReader.h>
#include <movetk/ds/ColumnarTrajectory.h>
#include <movetk/ds/TabularTrajectory.h>
#include <movetk/io/TrajectoryReader.h>
#include <movetk/io/ParseDate.h>


#include "movetk/OutlierDetection.h"
#include "movetk/io/CartesianProbeTraits.h"
#include "movetk/metric/Norm.h"
#include "movetk/outlierdetection/OutlierDetectionPredicates.h"
#include "movetk/utils/GeometryBackendTraits.h"
#include "movetk/utils/Iterators.h"
#include "movetk/utils/TrajectoryUtils.h"
class MyDateTime : public movetk::io::ParseDate{
public:
    MyDateTime(std::time_t timestamp=0): ParseDate(timestamp,"%Y-%m-%d %H:%M:%S"){}
};
// Define your probe type
// Define a description for your columns (for your own convenience)
enum ProbeColumns{
    id,
    Timestamp,
    Longitude,
    LAT
};
using ProbePoint = std::tuple<int, unsigned long int, double, double>;
using DataPoint = std::tuple<int, unsigned long int, double, double>;
using CsvReader = movetk::io::csv::csv< DataPoint,
                                        id,
                                        Timestamp,
                                        Longitude,
                                        LAT>;

// Does our file have a header
static constexpr bool has_header = true;
// The delimiter in the file that separates fields
static constexpr char delimiter = ',';
// Define the traits for the reader
using ProbeTraits = movetk::io::_ProbeTraits<ProbeColumns,CsvReader,ProbePoint,has_header,delimiter>;
//ProbeParseDate (std::time_t ts=0, std::string date_format="%Y-%m-%d %H:%M:%S")
using trajectory_type = movetk::utils::transfer_types<ProbePoint, movetk::ds::TabularTrajectory>;

//using tabular_trajectory_type = movetk::ds::TabularTrajectory<double, double, double, std::time_t, std::string, std::string>;
int main(int argc, char** argv){

    std::string file_name(argv[1]);
    std::unique_ptr<movetk::io::ProbeReader<ProbeTraits>> probe_reader;
    probe_reader = movetk::io::ProbeReaderFactory::create<ProbeTraits>(file_name);
    std::cerr << "opening file" << std::endl; 

    using Trajectory = movetk::ds::TabularTrajectoryForProbeType<ProbePoint>;
    static constexpr int SplitByFieldIdx = ProbeColumns::id;
    static constexpr unsigned long int SortByFieldIdx = ProbeColumns::Timestamp;
    using TrajectoryTraits = movetk::io::_TrajectoryTraits<ProbeTraits, SplitByFieldIdx, SortByFieldIdx, Trajectory>;
    using ProbeInputIterator = typename ProbeTraits::ProbeInputIterator;
    movetk::io::TrajectoryReader<TrajectoryTraits, ProbeInputIterator> trajectory_reader(probe_reader->begin(), probe_reader->end());
    std::cerr << "read file" << std::endl;

    using MovetkGeometryKernel = typename GeometryKernel::MovetkGeometryKernel;
    using CartesianProbeTraits = movetk::io::ProbeTraits<MovetkGeometryKernel>;
    using Probe = CartesianProbeTraits::ProbePoint;
    using Norm = typename GeometryKernel::Norm;
    using traj = std::vector<CartesianProbeTraits::ProbePoint>;
    using trajs = std::vector<traj>;
    using OutlierDetectionTraits = movetk::outlierdetection::OutlierDetectionTraits<CartesianProbeTraits, MovetkGeometryKernel, Norm>;
    using Test = movetk::outlierdetection::TEST<movetk::outlierdetection::linear_speed_bounded_test_tag, movetk::algo::cartesian_coordinates_tag, OutlierDetectionTraits>;
    static_assert(std::is_same_v<typename Test::NT, long double>);
    const auto test_val = static_cast<long double>(0.5);
    // Test test(test_val);
    using OutlierDetector = movetk::outlierdetection::OutlierDetection<OutlierDetectionTraits, Test, movetk::outlierdetection::output_sensitive_outlier_detector_tag>;
    movetk::geom::MakePoint<MovetkGeometryKernel> make_point;
    int total_outliers = 0;
    int n_traj = 0;
    std::cout << "id,time,latitude,longitude" << std::endl;

    for (const auto& trajectory : trajectory_reader){
        n_traj++;
        std::vector<int> ids = trajectory.get<id>();
        std::vector<double> lat = trajectory.get<LAT>();
        std::vector<unsigned long int> dt = trajectory.get<Timestamp>();
        std::vector<double> lon = trajectory.get<Longitude>();
        // std::cout << "Trajectory size " << trajectory.size() << std::endl;

        traj t;
        for (int i = 0; i < trajectory.size(); i++)
        {
            // std::cout << ids[i] << "," << lat[i] << "," << lon[i] << "," << dt[i] <<std::endl;
            t.push_back(std::make_tuple(make_point({lon[i], lat[i]}), dt[i]));
        }
        // std::cout << "Trajectory reconstructed"<< std::endl;
        
        // std::vector<Trajectory::const_iterator> result;

        // namespace od = movetk::outlierdetection;

        std::vector<traj::const_iterator> result;
        float speed = std::stof(argv[2]);
        OutlierDetector outlier_detector(speed * 0.000000001 / 111139);
        outlier_detector(std::cbegin(t), std::cend(t), std::back_inserter(result));
        std::reverse(std::begin(result), std::end(result));
        std::size_t num_outliers = t.size() - result.size();
        total_outliers+= num_outliers;
        // std::cout << "traj:" << ids[0] << ", initial: " << t.size() << ", normal: " << result.size()<< ", outliers: " << num_outliers;
        // std::cout << "\n";
        for (auto it : result)
        {
            std::cout << ids[0] << ",";
            std::cout << std::get<1>(*it)<<',';
            std::cout << std::get<0>(*it);
		    std::cout << "\n";
            // std::cout << 2 << std::endl;
        }

        t.clear();
    }
    // std::cout << "number of outliers "<< total_outliers << std::endl;
    // std::cout << "number of trajectories "<< n_traj << std::endl;
    return 0;
}


