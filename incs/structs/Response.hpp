/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hshimizu <hshimizu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 08:27:36 by hshimizu          #+#    #+#             */
/*   Updated: 2025/05/19 21:46:47 by hshimizu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "structs/Headers.hpp"

#include <map>
#include <string>
#include <vector>

struct Response {
  std::string version;
  int status;
  std::string reason;
  Headers headers;

  void swap(Response &rhs) throw();
};

void composeResponse(std::string &res, Response const &response);
