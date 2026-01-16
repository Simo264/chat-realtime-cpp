#pragma once

#include <array>
#include <atomic>
#include <shared_mutex>
#include <string_view>

#include <auth_service.grpc.pb.h>
#include <auth_service.pb.h>

#include "../../common.hpp"

/**
 * @class AuthServiceImpl
 * @brief Implementazione server del servizio di autenticazione.
 *
 * @details
 * Implementa le procedure RPC di login e signup definite in
 * `auth_service::AuthService`. Si occupa della validazione degli input, della
 * gestione concorrente del database utenti mediante `std::shared_mutex` e
 * dell'assegnazione atomica di nuovi `ClientID`. I metodi RPC restituiscono
 * `grpc::Status` per segnalare il risultato dell'operazione al client.
 */
class AuthServiceImpl : public auth_service::AuthService::Service
{
public:
  AuthServiceImpl();

  [[nodiscard]] grpc::Status
  LoginProcedure(grpc::ServerContext *context,
                 const auth_service::AuthRequest *request,
                 auth_service::AuthResponse *response) override;

  [[nodiscard]] grpc::Status
  SignupProcedure(grpc::ServerContext *context,
                  const auth_service::AuthRequest *request,
                  auth_service::AuthResponse *response) override;

private:
  bool find_user_record_by_username(
      std::string_view in_username, ClientID &out_userid,
      std::array<char, max_len_password> &out_password) const;

  bool validate_username(
      std::string_view username,
      std::array<char, max_len_error_message> &error_message) const;

  bool validate_password(
      std::string_view password,
      std::array<char, max_len_error_message> &error_message) const;

  void create_user_db(ClientID client_id, std::string_view username,
                      std::string_view password);

  // shared mutex: multipli lettori e un singolo scrittore
  std::shared_mutex m_db_users_mutex;
  // protezione semplice per l'aggiornamento del contatore
  std::atomic<ClientID> m_next_client_id;
};
