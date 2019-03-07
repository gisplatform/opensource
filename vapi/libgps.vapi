/* libgps.vapi
 *
 * Copyright (C) 2011 Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

[CCode (lower_case_cprefix = "gps_", gir_namespace = "Gps", gir_version = "2.0", cheader_filename = "gps.h")]
namespace Gps {

    /* delegates */
    [CCode (has_target = false)]
    public delegate void RawHookFunc( Device device, uint8[] data );

    /* enums and constants */
    [CCode (cname = "uint", cprefix = "WATCH_", cheader_filename = "gps.h")]
    public enum StreamingPolicy
    {
        ENABLE,
        JSON,
        NMEA,
        RARE,
        RAW,
        SCALED,
        NEWSTYLE,
        OLDSTYLE,
        DEVICE,
        DISABLE,
        SUBFRAMES
    }
    [CCode (cheader_filename = "gps.h")]
    public const uint POLL_NONBLOCK;
    [CCode (cheader_filename = "gps.h")]
    public const uint MAXTAGLEN;
    [CCode (cheader_filename = "gps.h")]
    public const uint MAXCHANNELS;
    [CCode (cheader_filename = "gps.h")]
    public const uint GPS_PRNMAX;
    [CCode (cheader_filename = "gps.h")]
    public const uint GPS_PATH_MAX;
    [CCode (cheader_filename = "gps.h")]
    public const uint GPS_BUFFER_MAX;
    [CCode (cheader_filename = "gps.h")]
    public const uint MAXUSERDEVS;

    [CCode (cname = "uint", cprefix = "", cheader_filename = "gps.h")]
    public enum ChangeMask
    {
        ONLINE_SET,
        TIME_SET,
        TIMERR_SET,
        LATLON_SET,
        ALTITUDE_SET,
        SPEED_SET,
        TRACK_SET,
        CLIMB_SET,
        STATUS_SET,
        MODE_SET,
        DOP_SET,
        VERSION_SET,
        HERR_SET,
        VERR_SET,
        ATTITUDE_SET,
        POLICY_SET,
        SATELLITE_SET,
        RAW_SET,
        USED_SET,
        SPEEDERR_SET,
        TRACKERR_SET,
        CLIMBERR_SET,
        DEVICE_SET,
        DEVICELIST_SET,
        DEVICEID_SET,
        ERROR_SET,
        RTCM2_SET,
        RTCM3_SET,
        AIS_SET,
        PACKET_SET,
        SUBFRAME_SET,
        AUXDATA_SET
    }

    [CCode (cname = "uint", cprefix = "STATUS_", cheader_filename = "gps.h")]
    public enum FixStatus
    {
        NO_FIX,
        FIX,
        DGPS_FIX,
    }

    [CCode (cname = "uint", cprefix = "", cheader_filename = "gps.h")]
    public enum FixMode
    {
        MODE_NOT_SEEN,
        MODE_NO_FIX,
        MODE_2D,
        MODE_3D
    }

    /* static functions */
    public static unowned string errstr(int errno);
    public static unowned string maskdump(uint64 set);
    public static void enable_debug(int level, Posix.FILE file);

    /* fix */
    [CCode (cname = "struct gps_fix_t", destroy_function = "", cprefix = "", cheader_filename = "gps.h")]
    public struct Fix
    {
        public double time;
        public FixMode mode;
        public double ept;
        public double latitude;
        public double epy;
        public double longitude;
        public double epx;
        public double altitude;
        public double epv;
        public double track;
        public double epd;
        public double speed;
        public double eps;
        public double climb;
        public double epc;
    }

    /* dilution of precision */
    [CCode (cname = "struct dop_t", destroy_function = "", cprefix = "", cheader_filename = "gps.h")]
    public struct Dop
    {
        public double xdop;
        public double ydop;
        public double pdop;
        public double hdop;
        public double vdop;
        public double tdop;
        public double gdop;
    }

	/* XXX SS: Attitude. */
	[CCode (cname = "attitude_t", destroy_function = "", cprefix = "", cheader_filename = "gps.h")]
	public struct Attitude
	{
		double heading;
		double pitch;
		double roll;
		double yaw;
		double dip;
		double mag_len; /* unitvector sqrt(x^2 + y^2 +z^2) */
		double mag_x;
		double mag_y;
		double mag_z;
		double acc_len; /* unitvector sqrt(x^2 + y^2 +z^2) */
		double acc_x;
		double acc_y;
		double acc_z;
		double gyro_x;
		double gyro_y;
		double temp;
		double depth;
		/* compass status -- TrueNorth (and any similar) devices only */
		char mag_st;
		char pitch_st;
		char roll_st;
		char yaw_st;
	}

    /* XXX SS: Navdata. */
    [CCode (cname = "struct devconfig_t", destroy_function = "", cprefix = "", cheader_filename = "gps.h")]
    public struct Devconfig
    {
      char path[];
      int flags;
      char driver[];
      char subtype[];
      double activated;
      uint stopbits;	/* RS232 link parameters */
      uint baudrate;	/* RS232 link parameters */
      char parity;			/* 'N', 'O', or 'E' */
      double cycle;     	/* refresh cycle time in seconds */
      double mincycle;     	/* refresh cycle time in seconds */
      int driver_mode;    		/* is driver in native mode or not? */
    }

    /* XXX SS: Navdata. */
    [CCode (cname = "struct navdata_t", destroy_function = "", cprefix = "", cheader_filename = "gps.h")]
    public struct Navdata
    {
      uint version;
      double compass_heading;
      double compass_deviation;
      double compass_variation;
      double air_temp;
      double air_pressure;
      double water_temp;
      double depth;
      double depth_offset;
      double wind_speed;
      double wind_dir;
      double crosstrack_error;
      uint compass_status;
      uint log_cumulative;
      uint log_trip;
      uint crosstrack_status;
    }

    /* device */
    [CCode (cname = "struct gps_data_t", destroy_function = "", cprefix = "gps_", cheader_filename = "gps.h")]
    public struct Device
    {
        public int gps_fd;
        public uint64 @set;
        public double online;
        public Fix fix;
        public double separation;
        public FixStatus status;
        public int satellites_used;
        public int used[];
        public Dop dop;
        public double epe;
        public double skyview_time;
        public int satellites_visible;
        public int PRN[];
        public int elevation[];
        public int azimuth[];
        public double ss[];
        public Attitude attitude;
        public Navdata navdata;
        public Devconfig dev;	/* device that shipped last update */

        [CCode (cname = "GPSD_SHARED_MEMORY")]
        public const string SHARED_MEMORY;

        [CCode (cname = "gps_open", instance_pos = -1)]
        public int open(string server = "localhost", string port = "2947");
        public void close();

        [PrintfFormat]
        public size_t send( string format, ... );
        public size_t read();
        public bool waiting(int timeout);
        public int stream( StreamingPolicy flags, void* data = null );
        public void set_raw_hook( RawHookFunc func );
    }
}


#if 0


extern void gps_clear_fix(/*@ out @*/struct gps_fix_t *);
extern void gps_merge_fix(/*@ out @*/struct gps_fix_t *,
			  gps_mask_t,
			  /*@ in @*/struct gps_fix_t *);
extern time_t mkgmtime(register struct tm *);
extern double timestamp(void);
extern double iso8601_to_unix(char *);
extern /*@observer@*/char *unix_to_iso8601(double t, /*@ out @*/char[], size_t len);
extern double gpstime_to_unix(int, double);
extern void unix_to_gpstime(double, /*@out@*/int *, /*@out@*/double *);
extern double earth_distance(double, double, double, double);
extern double earth_distance_and_bearings(double, double, double, double,
					  /*@null@*//*@out@*/double *,
					  /*@null@*//*@out@*/double *);
extern double wgs84_separation(double, double);


#endif

