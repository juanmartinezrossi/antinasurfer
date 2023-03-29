/** 
 * @mainpage ALOE Software Daemons Implementation Documentation
 *
 * This documentation covers the ALOE Software Daemons Implementations.
 * Software Daemons realize common background management tasks required by the whole 
 * framework. See ... for more help
 *
 * Go back to http://epsc-gcr-002.upc.es/flexlab/wiki/DeveloperGuide
 *
 *
 * Explicacio de com s'estructura un daemon, comandes, objectes comuns disponibles....
 *
 */



 /* here we define defgroup for common objects and commands as any file centralizes 
    such information 
  */


/** @defgroup common Daemons Common Objects
 *
 * Some common objects for dealing with strings, ids, packets, ALOE objects descriptions, and other utils
 * are defined in these group of functions 
 * @{
 */

/** @defgroup common_base Base Utils
 *
 * This group contains some common basic objects like Sets, Arrays, Lists, Buffers, etc. 
 */

/** @defgroup common_phal Basic Types
 *
 * This group contains basic data types used by most of the daemons like ALOE names (24-char strings), ALOE id's (32-bit integers),
 * packets, commands and other utils.
 *
 */

/** @defgroup common_obj ALOE Objects 
 * 
 * Here there are descriptions for ALOE Objects, ALOE Interfaces, Parameters, Variables, Applications, etc. used in the statistics and application level.
 *
 */

/** @} */

/** @defgroup commands Daemon Input Commands Reference
 *
 * In this section, the input commands each Daemon accepts from other daemons are documented.
 */


/** @defgroup sensors Daemon Background Tasks Documentation
 *
 * This section describes the Background tasks realized by Sensor Daemons (managers do not realize these tasks at all)
 */
