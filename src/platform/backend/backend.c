#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "internal.h"
#include "backend.h"
#ifdef ENABLE_X11_BACKEND
#include "x11.h"
#endif
#include "drm.h"

bool
wlc_backend_surface(struct wlc_backend_surface *surface, void (*destructor)(struct wlc_backend_surface*), size_t internal_size)
{
   assert(surface);
   memset(surface, 0, sizeof(struct wlc_backend_surface));

   if (internal_size > 0 && !(surface->internal = calloc(1, internal_size)))
      return false;

   surface->api.terminate = destructor;
   surface->internal_size = internal_size;
   return true;
}

void
wlc_backend_surface_release(struct wlc_backend_surface *surface)
{
   if (!surface)
      return;

   if (surface->api.terminate)
      surface->api.terminate(surface);

   if (surface->internal_size > 0 && surface->internal)
      free(surface->internal);

   memset(surface, 0, sizeof(struct wlc_backend_surface));
}

uint32_t
wlc_backend_update_outputs(struct wlc_backend *backend, struct chck_pool *outputs)
{
   assert(backend);

   if (!backend->api.update_outputs)
      return 0;

   return backend->api.update_outputs(outputs);
}

void
wlc_backend_release(struct wlc_backend *backend)
{
   if (!backend)
      return;

   if (backend->api.terminate)
      backend->api.terminate();

   memset(backend, 0, sizeof(struct wlc_backend));
}

bool
wlc_backend(struct wlc_backend *backend)
{
   assert(backend);
   memset(backend, 0, sizeof(struct wlc_backend));

   bool (*init[])(struct wlc_backend*) = {
#ifdef ENABLE_X11_BACKEND
      wlc_x11,
#endif
      wlc_drm,
      NULL
   };

   enum wlc_backend_type types[] = {
#ifdef ENABLE_X11_BACKEND
      WLC_BACKEND_X11,
#endif
      WLC_BACKEND_DRM,
   };

   for (uint32_t i = 0; init[i]; ++i) {
      if (init[i](backend)) {
         backend->type = types[i];
         return true;
      }
   }

   wlc_log(WLC_LOG_WARN, "Could not initialize any backend");
   return false;
}
