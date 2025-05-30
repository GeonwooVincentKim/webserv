/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hshimizu <hshimizu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/31 09:45:58 by hshimizu          #+#    #+#             */
/*   Updated: 2025/04/17 17:48:34 by hshimizu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <ftjson/Json.hpp>

#include <string>

struct Address {
  std::string host;
  int port;

  static int const portMin;
  static int const portMax;

  Address(std::string const &host, int port);
  Address(ftjson::Object const &addr);

  void swap(Address &rhs) throw();
  bool operator<(Address const &rhs) const;
};
