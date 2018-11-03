int   __________DEBUG__________    = 0;                                                                        // section dummy
bool  DBG_EN                       = 0;                                                                        // enable debug
bool  DBG_ORG_EN                   = 1;                                                                        // debug original image before any processing
bool  DBG_EQ_EN                    = 0;                                                                        // debug equalization before registration
bool  DBG_RGB_EN                   = 1;                                                                        // debug RGB output during merge
bool  DBG_WARP_EN                  = 0;                                                                        // debug backward warping
bool  DBG_DIFF_EN                  = 0;                                                                        // debug image current and base difference
bool  DBG_BLD_RTO_EN               = 0;                                                                        // final multi-frame blending ratio
bool  DBG_NLM_EN                   = 0;                                                                        // debug non-local mean filtering

int   _______FLT_SETTINGS______    = 0;                                                                        // section dummy
bool  CFG_REGISTRATION_EN          = 1;                                                                        // enable or disable image registration
bool  CFG_IMG_BRACKET_EN           = 0;                                                                        // enable bracket exposure for HDR imaging
int   CFG_IMG_MODE                 = 0;                                                                        // image output mode, 0: linear gain, 1: global curve mapping, 2: local tone mapping
bool  CFG_BLD_SKIP_EN              = 0;                                                                        // enable skipping image when large occlusion
bool  CFG_PRE_NR_EN                = 0;                                                                        // enable pre NR before MFNR
bool  CFG_POST_NR_EN               = 0;                                                                        // enable post NR at final image

int   _______IMG_PARAMS________    = 0;
float CFG_IMG_LINEAR_GAIN          = 1.0;                                                                      // image linear luminance gain   (CFG_IMG_MODE = 0)
float CFG_IMG_LINEAR_OFFSET        = 100.0;                                                                    // image linear luminance offset (CFG_IMG_MODE = 1)
float CFG_IMG_COLOR_GAIN_R         = 1.0;                                                                      // output color correction gain  (1.0 as no correction)
float CFG_IMG_COLOR_GAIN_G         = 1.0;                                                                      // output color correction gain  (1.0 as no correction)
float CFG_IMG_COLOR_GAIN_B         = 1.0;                                                                      // output color correction gain  (1.0 as no correction)

int   ______REGISTRATION_______    = 0;                                                                        // registration parameters
float CFG_ECC_SCALE                = 0.125;                                                                    // image registration scaling ratio
int   CFG_ECC_ITER                 = 1000;                                                                     // image registration max iteration number
float CFG_ECC_EPS                  = 0.0001;                                                                   // image registration convergence epsilon

int   ________DIFF_MAP_________    = 0;                                                                        // section dummy
int   CFG_DIF_MAP_PRE_MED_SIZE     = 3;                                                                        // image pre median filter
int   CFG_DIF_MAP_FB_SIZE          = 15;                                                                       // image pre fast bilateral filter kernel
float CFG_DIF_MAP_FB_TH            = 10;                                                                       // image pre fast bilateral filter threshold
int   CFG_DIF_MAP_DILATE_SIZE      = 5;                                                                        // diff mask control
int   CFG_DIF_MAP_ERODE_SIZE       = 0;                                                                        // diff mask control
int   CFG_DIF_MAP_POST_MED_SIZE    = 3;                                                                        // post diff mask median filter

int   ________BLENDING_________    = 0;                                                                        // section dummy
float CFG_BLD_DIF_ROI_RTO          = 5;                                                                        // average pixel difference ROI ratio
float CFG_BLD_DIF_AVG_TH           = 15;                                                                       // average pixel difference threshold, if > th, skip blending this frame
float CFG_BLD_RTO_X_0              = 10;                                                                       // pixel blending diff threshold
float CFG_BLD_RTO_X_1              = 40;                                                                       // pixel blending diff threshold
float CFG_BLD_RTO_Y_0              = 0.5;                                                                      // pixel blending blending ratio
float CFG_BLD_RTO_Y_1              = 0.0;                                                                      // pixel blending blending ratio

int   _________PRE__NR_________    = 0;                                                                        // section dummy
int   CFG_PRE_NR_MED_WIN_SIZE_R    = 7;                                                                        // pre NR median filtering window size
int   CFG_PRE_NR_MED_WIN_SIZE_G    = 0;                                                                        // pre NR median filtering window size
int   CFG_PRE_NR_MED_WIN_SIZE_B    = 7;                                                                        // pre NR median filtering window size
int   CFG_PRE_NR_MED_WIN_SIZE_Y    = 0;                                                                        // pre NR median filtering window size
int   CFG_PRE_NR_MED_WIN_SIZE_U    = 7;                                                                        // pre NR median filtering window size
int   CFG_PRE_NR_MED_WIN_SIZE_V    = 7;                                                                        // pre NR median filtering window size

int   _________POST_NR_________    = 0;                                                                        // section dummy
bool  CFG_POST_NR_NLM_EN           = 0;                                                                        // enable fast non-local mean filtering
bool  CFG_POST_NR_JBL_EN           = 0;                                                                        // enable joint fast bilateral filtering
bool  CFG_POST_NR_BL_EN            = 1;                                                                        // enable fast bilateral filtering
bool  CFG_POST_NR_HSV_EN           = 0;
float CFG_POST_NR_NLM_STRENGTH     = 5;                                                                        // fast non-local mean diff threshold
int   CFG_POST_NR_NLM_WIN_SIZE     = 5;                                                                        // fast non-local mean block matching window size
int   CFG_POST_NR_NLM_SEARCH_RANGE = 11;                                                                       // fast non-local mean local search range
float CFG_POST_NR_NLM_BLD_RTO_X_0  = 0;
float CFG_POST_NR_NLM_BLD_RTO_X_1  = 0.5;
float CFG_POST_NR_NLM_BLD_RTO_Y_0  = 0.8;
float CFG_POST_NR_NLM_BLD_RTO_Y_1  = 0.15;
int   CFG_POST_NR_JBL_WIN_SIZE     = 7;
float CFG_POST_NR_JBL_TH           = 10;
int   CFG_POST_NR_BL_WIN_SIZE      = 7;
float CFG_POST_NR_BL_TH            = 10;


int   _______AUTO_PARAMS_______    = 0;                                                                        // section dummy
int   CFG_IMG_NUM                  = 0;                                                                        // image number
int   CFG_IMG_IDX                  = 0;                                                                        // current process image index
int   CFG_IMG_SIZE_X               = 0;                                                                        // image width
int   CFG_IMG_SIZE_Y               = 0;                                                                        // image height
int   CFG_IMG_PIX_NUM              = 0;                                                                        // image width * height
int   CFG_MAX_PYD_LVL              = 0;                                                                        // maximum image pyramid level
