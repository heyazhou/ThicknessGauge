#pragma once

#include "CaptureInterface.h"
#include <PvApi.h>
#include "UI/ProgressBar.h"

/**
 * \brief Allows capture through PvAPI -> OpenCV data structure
 */
class CapturePvApi {

public:

    enum class PixelFormat {
        MONO8, MONO12, MONO12_PACKED, UNKNOWN
    };


private:

    const int mono = 1;

    const unsigned long def_packet_size = 8228;

    tg::tCamera camera_;

    tPvCameraInfo camera_info_;

    unsigned long frame_size_;

    unsigned int retry_count_;

    bool initialized_;

    bool is_open_;

    bool exposure_target_reached_;

    static char const* error_last(tPvErr error);

    static const char* data_type_to_string(tPvDatatype aType);

    void query_attribute(const char* aLabel) const;

public:

    const cv::Rect_<unsigned long> default_roi = cv::Rect_<unsigned long>(0, 1006, 2448, 256);

    CapturePvApi()
        : frame_size_(0), retry_count_(10), initialized_(false), is_open_(false), exposure_target_reached_(false) { }

    CapturePvApi(tg::tCamera myCamera, tPvCameraInfo cameraInfo, unsigned long frameSize)
        : camera_(myCamera),
          camera_info_(cameraInfo),
          frame_size_(frameSize), retry_count_(10), initialized_(true), is_open_(false), exposure_target_reached_(false) { }

    ~CapturePvApi() {
        delete[] static_cast<char*>(camera_.Frame.ImageBuffer);
    }

    bool frame_init();

    int cap_init() const;

    bool cap_end() const;

    bool aquisition_init() const;

    bool aquisition_end() const;

    auto exposure_auto_reset() {
        //--- /Controls/Exposure/Auto/ExposureAutoAlg = Mean [enum,rw  ]
        //--- /Controls/Exposure/Auto/ExposureAutoMax = 500000 [uint32,rw  ]
        //--- /Controls/Exposure/Auto/ExposureAutoMin = 25 [uint32,rw  ]

        const unsigned long auto_max = 500000;
        const unsigned long auto_min = 25;
        const std::string auto_alg = "Mean";

        using namespace tg;

        // set the defaults
        auto err_code = PvAttrUint32Set(camera_.Handle, "ExposureAutoMax", auto_max);
        if (err_code != ePvErrSuccess) {
            log_time << cv::format("Error.. ExposureAutoMax.. %s\n", error_last(err_code));
            return false;
        }
        err_code = PvAttrUint32Set(camera_.Handle, "ExposureAutoMin", auto_min);
        if (err_code != ePvErrSuccess) {
            log_time << cv::format("Error.. ExposureAutoMin.. %s\n", error_last(err_code));
            return false;
        }
        err_code = PvAttrEnumSet(camera_.Handle, "ExposureAutoAlg", auto_alg.c_str());
        if (err_code != ePvErrSuccess) {
            log_time << cv::format("Error.. ExposureAutoAlg.. %s\n", error_last(err_code));
            return false;
        }

        // check the defaults to make sure they are configured correctly
        unsigned long auto_max_check = 0;
        unsigned long auto_min_check = 0;
        char lValue[128];

        err_code = PvAttrUint32Get(camera_.Handle, "ExposureAutoMax", &auto_max_check);
        if (err_code != ePvErrSuccess) {
            log_time << cv::format("Error.. ExposureAutoMax.. %s\n", error_last(err_code));
            return false;
        }

        err_code = PvAttrUint32Get(camera_.Handle, "ExposureAutoMin", &auto_max_check);
        if (err_code != ePvErrSuccess) {
            log_time << cv::format("Error.. ExposureAutoMin.. %s\n", error_last(err_code));
            return false;
        }


        err_code = PvAttrEnumGet(camera_.Handle, "ExposureAutoAlg", lValue, 128, nullptr);
        if (err_code != ePvErrSuccess) {
            log_time << cv::format("Error.. ExposureAutoAlg.. %s\n", error_last(err_code));
            return false;
        }

        if (auto_max_check != auto_max) {
            log_time << cv::format("Error.. ExposureAutoMax mismatch.. %s\n", error_last(err_code));
            return false;
        }

        if (auto_max_check != auto_max) {
            log_time << cv::format("Error.. ExposureAutoMin mismatch.. %s\n", error_last(err_code));
            return false;
        }

        if (std::strcmp(auto_alg.c_str(), lValue) != 0) {
            log_time << cv::format("Error.. ExposureAutoAlg mismatch.. %s\n", error_last(err_code));
            return false;
        }

        return true;

    }

    //Controls/Exposure/Auto/ExposureAutoAdjustTol = 5 [uint32,rw  ]
    //Controls/Exposure/Auto/ExposureAutoOutliers = 0 [uint32,rw  ]
    //Controls/Exposure/Auto/ExposureAutoRate = 100 [uint32,rw  ]
    //Controls/Exposure/Auto/ExposureAutoTarget = 50 [uint32,rw  ]
    //Controls/Exposure/ExposureMode = Manual [enum,rwv ]

    void exposure_mode() { }

    bool exposure_auto_adjust_tolerance(unsigned long new_value) {

        auto err_code = PvAttrUint32Set(camera_.Handle, "ExposureAutoAdjustTol", new_value);

        if (err_code == ePvErrSuccess)
            return true;

        using namespace tg;
        
        log_time << cv::format("Error.. ExposureAutoAdjustTol.. %s\n", error_last(err_code));
        return false;

    }

    unsigned long exposure_auto_adjust_tolerance() {

        unsigned long ret_val = 0;
        
        auto err_code = PvAttrUint32Get(camera_.Handle, "ExposureAutoAdjustTol", &ret_val);

        if (err_code == ePvErrSuccess)
            return ret_val;
        
        using namespace tg;

        log_time << cv::format("Error.. ExposureAutoAdjustTol.. %s\n", error_last(err_code));
        return ret_val;

    }

    void exposure_auto_rate(unsigned long new_value);

    unsigned long exposure_auto_rate();

    void exposure_auto_target(unsigned long new_value);

    unsigned long exposure_auto_target();


    /**
     * \brief Resets binning
     */
    void reset_binning() const;

    bool is_open() const;

    void is_open(bool new_value);

    bool initialized() const;

    void initialized(bool new_value);

    unsigned retry_count() const;

    void retry_count(unsigned new_value);

    unsigned long frame_size() const;

    std::string version() const;

    /**
     * \brief Apply a specific ROI to the camera
     * \param new_region The region as opencv rect of unsigned long
     * \return true if all 4 regions were set without errors
     */
    bool region(cv::Rect_<unsigned long> new_region) const;

    /**
     * \brief Retrieves the camera ROI
     * \return The roi as opencv rect type unsigned long
     */
    cv::Rect_<unsigned long> region() const;

    bool region_x(unsigned new_x) const;

    unsigned long region_x() const;

    bool region_y(unsigned new_y) const;

    unsigned long region_y() const;

    bool region_height(unsigned new_height) const;

    unsigned long region_height() const;

    bool region_width(unsigned new_width) const;

    unsigned long region_width() const;

    /**
     * \brief Captures frames synchron into a vector of opencv matricies using specified exposure
     * \param frame_count Amount of frames to capture
     * \param target_vector The target vector for the captured images
     */
    void cap(int frame_count, std::vector<cv::Mat>& target_vector);

    bool initialize();

    void uninitialize();

    static unsigned long camera_count();

    bool open();

    void close();

    void packet_size(const unsigned long new_value) const;

    void gain(unsigned long new_value) const;

    unsigned long gain() const;

    void exposure(unsigned long new_value) const;

    unsigned long exposure() const;

    void exposure_add(unsigned long value_to_add) const;

    void exposure_sub(unsigned long value_to_sub) const;

    void exposure_mul(unsigned long value_to_mul) const;

    void pixel_format(const PixelFormat format) const;

    PixelFormat pixel_format() const;

    void print_attr() const;

};
