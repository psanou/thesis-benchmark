#include <tuple>
#include <movetk/io/csv/csv.h>
#include <movetk/io/ProbeTraits.h>
#include <movetk/io/ProbeReader.h>
#include <movetk/ds/ColumnarTrajectory.h>
#include <movetk/ds/TabularTrajectory.h>
#include <movetk/io/TrajectoryReader.h>
#include <movetk/io/ParseDate.h>

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
    Longitude,
    n
};
using ProbePoint = std::tuple<int, unsigned long int, double, double, bool>;
using DataPoint = std::tuple<int, unsigned long int, double, double, bool>;
using CsvReader = movetk::io::csv::csv< DataPoint,
                                            id,
    Timestamp,
    Latitude,
    Longitude,
    n>;

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
    std::string file_name(argv[1]);
    std::unique_ptr<movetk::io::ProbeReader<ProbeTraits>> probe_reader;
    probe_reader = movetk::io::ProbeReaderFactory::create<ProbeTraits>(file_name);
    // std::cout << "opening file" << std::endl; 

    using Trajectory = movetk::ds::TabularTrajectoryForProbeType<ProbePoint>;
    static constexpr int SplitByFieldIdx = ProbeColumns::id;
    static constexpr unsigned long int SortByTimestamp = ProbeColumns::Timestamp;
    using TrajectoryTraits = movetk::io::_TrajectoryTraits<ProbeTraits, SplitByFieldIdx, SortByTimestamp, Trajectory>;
    using ProbeInputIterator = typename ProbeTraits::ProbeInputIterator;
    movetk::io::TrajectoryReader<TrajectoryTraits, ProbeInputIterator,true,false> trajectory_reader(probe_reader->begin(), probe_reader->end());
    // std::cout << "id,Timestamp,Latitude,Longitude" << std::endl;

    for (const auto& trajectory : trajectory_reader){
        std::vector<int> ids = trajectory.get<id>();
        std::vector<double> lat = trajectory.get<Latitude>();
        std::vector<double> lon = trajectory.get<Longitude>();
        std::vector<unsigned long int> dt = trajectory.get<Timestamp>();
        std::vector<bool> value = trajectory.get<n>();

        std::cout << "Trajectory size " << trajectory.size() << std::endl;
        for (int i = 0; i < trajectory.size(); i++)
        {
            std::cout << ids[i]  << "," << dt[i] << "," << lat[i] << "," << lon[i] << "," << value[i] <<std::endl;
        }
        
    }
    return 0;
}


