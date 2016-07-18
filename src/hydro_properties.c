/*******************************************************************************
 * This file is part of SWIFT.
 * Copyright (c) 2016 Matthieu Schaller (matthieu.schaller@durham.ac.uk)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

/* This object's header. */
#include "hydro_properties.h"

/* Standard headers */
#include <float.h>
#include <math.h>

/* Local headers. */
#include "adiabatic_index.h"
#include "common_io.h"
#include "error.h"
#include "hydro.h"
#include "kernel_hydro.h"

#define hydro_props_default_max_iterations 30
#define hydro_props_default_volume_change 2.0f

void hydro_props_init(struct hydro_props *p,
                      const struct swift_params *params) {

  /* Kernel properties */
  p->eta_neighbours = parser_get_param_float(params, "SPH:resolution_eta");
  const float eta3 = p->eta_neighbours * p->eta_neighbours * p->eta_neighbours;
  p->target_neighbours = 4.0 * M_PI * kernel_gamma3 * eta3 / 3.0;
  p->delta_neighbours = parser_get_param_float(params, "SPH:delta_neighbours");

  /* Ghost stuff */
  p->max_smoothing_iterations = parser_get_opt_param_int(
      params, "SPH:max_ghost_iterations", hydro_props_default_max_iterations);

  /* Time integration properties */
  p->CFL_condition = parser_get_param_float(params, "SPH:CFL_condition");
  const float max_volume_change = parser_get_opt_param_float(
      params, "SPH:max_volume_change", hydro_props_default_volume_change);
  p->log_max_h_change = logf(powf(max_volume_change, 0.33333333333f));
}

void hydro_props_print(const struct hydro_props *p) {

  message("Adiabatic index gamma: %f.", hydro_gamma);
  message("Hydrodynamic scheme: %s.", SPH_IMPLEMENTATION);
  message("Hydrodynamic kernel: %s with %.2f +/- %.2f neighbours (eta=%f).",
          kernel_name, p->target_neighbours, p->delta_neighbours,
          p->eta_neighbours);
  message("Hydrodynamic integration: CFL parameter: %.4f.", p->CFL_condition);
  message(
      "Hydrodynamic integration: Max change of volume: %.2f "
      "(max|dlog(h)/dt|=%f).",
      powf(expf(p->log_max_h_change), 3.f), p->log_max_h_change);

  if (p->max_smoothing_iterations != hydro_props_default_max_iterations)
    message("Maximal iterations in ghost task set to %d (default is %d)",
            p->max_smoothing_iterations, hydro_props_default_max_iterations);
}

#if defined(HAVE_HDF5)
void hydro_props_print_snapshot(hid_t h_grpsph, const struct hydro_props *p) {

  writeAttribute_f(h_grpsph, "Adiabatic index", hydro_gamma);
  writeAttribute_s(h_grpsph, "Scheme", SPH_IMPLEMENTATION);
  writeAttribute_s(h_grpsph, "Kernel function", kernel_name);
  writeAttribute_f(h_grpsph, "Kernel target N_ngb", p->target_neighbours);
  writeAttribute_f(h_grpsph, "Kernel delta N_ngb", p->delta_neighbours);
  writeAttribute_f(h_grpsph, "Kernel eta", p->eta_neighbours);
  writeAttribute_f(h_grpsph, "CFL parameter", p->CFL_condition);
  writeAttribute_f(h_grpsph, "Volume log(max(delta h))", p->log_max_h_change);
  writeAttribute_f(h_grpsph, "Volume max change time-step",
                   powf(expf(p->log_max_h_change), 3.f));
  writeAttribute_f(h_grpsph, "Max ghost iterations",
                   p->max_smoothing_iterations);
}
#endif
