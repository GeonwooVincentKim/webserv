/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   KqueueSelector.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hshimizu <hshimizu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 17:23:36 by hshimizu          #+#    #+#             */
/*   Updated: 2025/03/30 01:46:39 by hshimizu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#if defined(__APPLE__) || defined(__FreeBSD__)

#include <ftpp/selectors/Selector.hpp>

namespace ftpp {

class KqueueSelector : public Selector {
public:
  static int const max_events;

private:
  int _kq;

  static int _create_kqueue();

public:
  KqueueSelector();
  ~KqueueSelector();

  void add(int fd, event_t events);
  void remove(int fd);
  void modify(int fd, event_t events);
  void select(Events &events, int timeout) const;
};

} // namespace ftpp

#endif
