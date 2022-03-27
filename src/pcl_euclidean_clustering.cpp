#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/common/common.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/passthrough.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/kdtree/kdtree.h>
#include <pcl/segmentation/extract_clusters.h>
#include <lidar_based_tracking_object/SegmentedClustersArray.h>
#include <visualization_msgs/Marker.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <boost/lexical_cast.hpp>
#include <pcl/io/pcd_io.h>
#include <iostream>

void GetAxisAlignedBoundingBox(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud,
                               geometry_msgs::Pose* pose,
                               geometry_msgs::Vector3* dimensions) {
  Eigen::Vector4f min_pt, max_pt;
  pcl::getMinMax3D(*cloud, min_pt, max_pt);

  pose->position.x = (max_pt.x() + min_pt.x()) / 2;
  pose->position.y = (max_pt.y() + min_pt.y()) / 2;
  pose->position.z = (max_pt.z() + min_pt.z()) / 2;
  pose->orientation.w = 1;

  dimensions->x = max_pt.x() - min_pt.x();
  dimensions->y = max_pt.y() - min_pt.y();
  dimensions->z = max_pt.z() - min_pt.z();

  ROS_INFO("%f %f %f %f %f %f", pose->position.x, pose->position.y, pose->position.z, dimensions->x, dimensions->y, dimensions->z);
}

class cloudHandler
{
public:
    cloudHandler()
    {
        pcl_sub = nh.subscribe("pcl_segmented", 10, &cloudHandler::cloudCB, this);
        pcl_pub = nh.advertise<sensor_msgs::PointCloud2>("pcl_cluster1", 1);
        pcl_markerpub = nh.advertise<visualization_msgs::Marker>("visualization_marker", 100);
        // pcl_clusterpub = nh.advertise<lidar_based_tracking_object::SegmentedClustersArray>("pcl_cluster2", 1);
    }



    void cloudCB(const sensor_msgs::PointCloud2 &input)
    {
        pcl::PointCloud<pcl::PointXYZ> cloud;
        // pcl::PointCloud<pcl::PointXYZ> cloud_clustered;
        //pcl::PointCloud<pcl::PointXYZ> cluster;

        pcl::fromROSMsg(input, cloud);

        // Create the KdTree object for the search method of the extraction
        pcl::search::KdTree<pcl::PointXYZ>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZ>);
        tree->setInputCloud (cloud.makeShared());

        // create the extraction object for the clusters
        std::vector<pcl::PointIndices> cluster;
        pcl::EuclideanClusterExtraction<pcl::PointXYZ> ec;
        // specify euclidean cluster parameters
        ec.setClusterTolerance (1); // 2cm
        ec.setMinClusterSize (10);
        ec.setMaxClusterSize (50);
        ec.setSearchMethod (tree);
        ec.setInputCloud (cloud.makeShared());
        // exctract the indices pertaining to each cluster and store in a vector of pcl::PointIndices
        ec.extract (cluster);

        // boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer_cluster(new pcl::visualization::PCLVisualizer("extracted clusters"));
        // viewer_cluster->setBackgroundColor(0, 0, 0);

        // viewer_cluster->addCoordinateSystem(1.0);
        // viewer_cluster->initCameraParameters();
        // viewer_cluster->setRepresentationToWireframeForAllActors();
        // std::vector<ros::Publisher> pub_pcl_vec;
        // std::vector<ros::Publisher> pub_marker_vec;
        
        // for (int i = 0; i < cluster.size(); ++i)
        // {
        //     std::string topicNameCluster = "pcl_cluster" + boost::lexical_cast<std::string>(i);
        //     std::string topicNameMarker = "visualization_marker" + boost::lexical_cast<std::string>(i);
        //     pcl_markerpub = nh.advertise<visualization_msgs::Marker>(topicNameMarker, 100);
        //     pcl_pub = nh.advertise<sensor_msgs::PointCloud2>(topicNameCluster, 1);
        //     pub_pcl_vec.push_back(pcl_pub);
        //     pub_marker_vec.push_back(pcl_markerpub);
        // }
        
        pcl::PCLPointCloud2 outputPCL;
        // pcl::PCDWriter writer;
        // lidar_based_tracking_object::SegmentedClustersArray CloudClusters;
        sensor_msgs::PointCloud2 output;
        int cluster_i=0;
        // int j = 0;
        // int currentClusterNum = 1;
        // int count = 0;
        // here, cluster_indices is a vector of indices for each cluster. iterate through each indices object to work with them seporately
        for (std::vector<pcl::PointIndices>::const_iterator it = cluster.begin (); it != cluster.end (); ++it)
        {

            // create a new clusterData message object
            //obj_recognition::ClusterData clusterData;


            // create a pcl object to hold the extracted cluster
            pcl::PointCloud<pcl::PointXYZ> *cluster = new pcl::PointCloud<pcl::PointXYZ>;
            pcl::PointCloud<pcl::PointXYZ>::Ptr clusterPtr (cluster);

            // now we are in a vector of indices pertaining to a single cluster.
            // Assign each point corresponding to this cluster in xyzCloudPtrPassthroughFiltered a specific color for identification purposes
            for (std::vector<int>::const_iterator pit = it->indices.begin (); pit != it->indices.end (); ++pit)
            
            //     // clusterPtr->points[*pit].r = CLUSTER_COLOR[cluster_i][RED];
            //     // clusterPtr->points[*pit].g = CLUSTER_COLOR[cluster_i][GREEN];
            //     // clusterPtr->points[*pit].b = CLUSTER_COLOR[cluster_i][BLUE];
            
                clusterPtr->points.push_back(cloud.makeShared()->points[*pit]);
            // std::string fileName = "/home/szeqi/catkin_ws/src//lidar_based_tracking_object/writePCD/cluster" + boost::to_string(currentClusterNum) + ".pcd";
		    // pcl::io::savePCDFileASCII(fileName, *clusterPtr);
            // currentClusterNum++;
            // std::stringstream ss;
            // ss << "cloud_cluster_pcl" << j << ".pcd";
            // writer.write<pcl::PointXYZ>(ss.str(), *clusterPtr, false); //*
            // pcl::PointXYZ minPt, maxPt;

            // pcl::getMinMax3D(*clusterPtr, minPt, maxPt);
            // std::cout << "Max x: " << maxPt.x << std::endl;
            // std::cout << "Max y: " << maxPt.y << std::endl;
            // std::cout << "Max z: " << maxPt.z << std::endl;
            // std::cout << "Min x: " << minPt.x << std::endl;
            // std::cout << "Min y: " << minPt.y << std::endl;
            // std::cout << "Min z: " << minPt.z << std::endl;
            // float x_min = minPt.x;
            // float x_max = maxPt.x;
            // float y_min = minPt.y;
            // float y_max = maxPt.y;
            // float z_min = minPt.z;
            // float z_max = maxPt.z;
            // viewer_cluster->setRepresentationToWireframeForAllActors();
            // viewer_cluster->addCube(x_min, x_max, y_min, y_max, z_min, z_max, 1.0, 0, 0, std::to_string(j+50), 0);
            // viewer_cluster->setRepresentationToWireframeForAllActors();
            // viewer_cluster->addPointCloud<pcl::PointXYZ>(clusterPtr, std::to_string(j));
            // viewer_cluster->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, std::to_string(j));
            // viewer_cluster->setRepresentationToWireframeForAllActors();
            // //std::cout << "PointCloud representing the Cluster: " << cloud_cluster->points.size() << " data points." << std::endl;
            // j++;

            cluster_i++;
            visualization_msgs::Marker object_marker;
            object_marker.ns = "objects";
            object_marker.id = cluster_i;
            ROS_INFO("object marker id : %d", object_marker.id);
            object_marker.header.frame_id = "odom";
            object_marker.type = visualization_msgs::Marker::CUBE;
            GetAxisAlignedBoundingBox(clusterPtr, &object_marker.pose, &object_marker.scale);
            // object_marker.pose = pose;
            // object_marker.scale = dimensions;
            object_marker.color.g = 1;
            object_marker.color.a = 0.3;
            pcl_markerpub.publish(object_marker);
            ROS_INFO("%d", cluster_i);


            // cluster_i++;
            // if(cluster_i >= CLUSTER_MAX){
            //     break;
            // count++;
            // printf("%d", count);
            //convert to pcl::PCLPointCloud2
            pcl::toPCLPointCloud2( *clusterPtr ,outputPCL);

            

            //Convert to ROS data type
            pcl_conversions::fromPCL(outputPCL, output);

            

            // add the cluster to the array message
            //clusterData.cluster = output;
            // CloudClusters.clusters.push_back(output);
            output.header.frame_id = "odom";
            pcl_pub.publish(output);
            // pub_pcl_vec[j].publish (output);
            // pub_marker_vec[j].publish(object_marker);
            // j++;
        }
        // viewer_cluster->spinOnce(5000);
        // viewer_cluster->setRepresentationToWireframeForAllActors();
        // viewer_cluster->close();
        // pcl_clusterpub.publish(CloudClusters);
    }
        
protected:
    ros::NodeHandle nh;
    ros::Subscriber pcl_sub;
    ros::Publisher pcl_pub;
    ros::Publisher pcl_markerpub;
    // // ros::Publisher pcl_clusterpub;
    // ros::Publisher pub_marker_vec;
    // ros::Publisher pub_pcl_vec;
};

main(int argc, char **argv)
{
    ros::init(argc, argv, "pcl_euclidean_clustering");

    cloudHandler handler;

    ros::spin();

    return 0;
}

