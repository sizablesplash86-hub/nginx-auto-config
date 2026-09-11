#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "/mnt/code-projects/packages/auto-config/version 3.0/config.h"

void pingora(void)
{
  // REMOVE THIS SECTION
  char ent;
  printf("The Pingora auto config is in testing and not fully setup yet. Would you like to continue? ");
  scanf(" %c", &ent);
  if (ent == 'n')
  {
    return;
  }  //

  char ping;
  if (system("ls -d /etc/pingora > /dev/null 2>&1") != 0)
  {
    printf("Pingora not installed. Would you like to install? y/n: ");
    scanf(" %c", &ping);

    if (ping == 'y')
    {

      printf("[*] Installing pingora and dependencies\n");
      if (system("sudo apt update && sudo apt install build-essential clang perl libssl-dev pkg-config curl git cmake -y") != 0)
      {
        printf("\033[31mERROR\033[0m Failed to install dependencies. Exiting now...\n");
        return;
      }

      printf("[*] Installing rust...\n");
      if (system("curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y") != 0)
      {
        printf("\033[31mERROR\033[0m Failed to install rust. Exiting now...\n\n");
        return;
      }

      printf("[*] Creating directories...\n");
      if (system(". $HOME/.cargo/env && cargo new --bin /etc/pingora --quiet") != 0)
      {
        printf("\033[31mERROR\033[0m Failed to create directory. Exiting now...\n\n");
        return;
      }

      if (system("mkdir -p /var/log/pingora && mkdir -p /etc/pingora/sites") != 0)
      {
        printf("\033[31mERROR\033[0m Failed to create directory. Exiting now...\n\n");
        return;
      }

      FILE *fp = fopen("/etc/pingora/Cargo.toml", "w");
      fprintf(fp,
"[package]\n"
        "name = \"proxy_engine\"\n"
        "version = \"0.1.0\"\n"
        "edition = \"2021\"\n\n"
        "[dependencies]\n"
        "async-trait = \"0.1\"\n"
        "pingora = { version = \"0.8\", features = [\"lb\"] }\n"
        "pingora-core = \"0.8\"\n"
        "pingora-proxy = \"0.8\"\n"
        "env_logger = \"0.11\"\n"
        "log = \"0.4\"\n"
      );
      fclose(fp);

      FILE *rs = fopen("/etc/pingora/src/main.rs", "w");
      fprintf(rs,
        "use async_trait::async_trait;\n"
        "use pingora_core::server::Server;\n"
        "use pingora_core::upstreams::peer::HttpPeer;\n"
        "use pingora_proxy::{ProxyHttp, Session};\n\n"
        "pub struct MyProxy;\n\n"
        "#[async_trait]\n"
        "impl ProxyHttp for MyProxy {\n"
        "    type CTX = ();\n"
        "    fn new_ctx(&self) -> Self::CTX {}\n\n"
        "    async fn request_filter(&self, session: &mut Session, _ctx: &mut Self::CTX) -> pingora_core::Result<bool> {\n"
        "        let req = session.req_header();\n"
        "        println!(\"==> Proxy intercepted: {} {}\", req.method, req.uri);\n"
        "        Ok(false)\n"
        "    }\n\n"
        "    async fn upstream_peer(&self, _session: &mut Session, _ctx: &mut Self::CTX) -> pingora_core::Result<Box<HttpPeer>> {\n"
        "        let peer = Box::new(HttpPeer::new(\"127.0.0.1:3519\", false, \"localhost\".to_string()));\n"
        "        Ok(peer)\n"
        "    }\n"
        "}\n\n"
        "fn main() {\n"
        "    env_logger::init_from_env(env_logger::Env::default().default_filter_or(\"info\"));\n"
        "    let mut my_server = Server::new(None).unwrap();\n"
        "    my_server.bootstrap();\n\n"
        "    let mut my_proxy = pingora_proxy::http_proxy_service(&my_server.configuration, MyProxy);\n"
        "    my_proxy.add_tcp(\"0.0.0.0:6188\");\n\n"
        "    my_server.add_service(my_proxy);\n"
        "    my_server.run_forever();\n"
        "}\n"
      );
      fclose(rs);

      printf("[*] Compiling optimized release binary...\n");
      if (system(". $HOME/.cargo/env && cd /etc/pingora && cargo build --release") != 0)
      {
        printf("\033[31mERROR\033[0m Rust build compilation failed. Exiting now...\n\n");
        system("rm -rf /var/log/pingora && rm -rf /etc/pingora");
        return;
      }

      printf("[*] Creating background monitor...\n");
      FILE *sy = fopen("/etc/systemd/system/pingora.service", "w");
      fprintf(sy,
"[Unit]\n"
        "Description=Pingora Custom High-Performance Gateway Proxy\n"
        "After=network.target\n\n"
        "[Service]\n"
        "Type=simple\n"
        "User=root\n"
        "Environment=RUST_LOG=info\n"
        "WorkingDirectory=/etc/pingora/\n"
        "ExecStart=/etc/pingora/target/release/proxy_engine\n"
        "Restart=always\n"
        "RestartSec=5\n\n"
        "[Install]\n"
        "WantedBy=multi-user.target\n"
      );
      fclose(sy);

      system("sudo systemctl daemon-reload");
      system("sudo systemctl enable pingora.service --quiet");
      if (system("sudo systemctl start pingora.service") !=0)
      {
        printf("pingora failed to install.");
        system("rm -rf /var/log/pingora && rm -rf /etc/pingora");
        return;
      }
      printf("Pingora sucessfully installed!\n\n");
      return;
    }

    else
    {
      return;
    }
  }
  char ins;
  printf("Pingora installed!\n\nWould you like to list options? y/n:  ");
  scanf(" %c", &ins);

  if (ins == 'y')
  {
    printf("\nWeb server options:\n1) Presets\n2) manual proxy/directory\n\nPingora system:\n3) Pingora status\n4) Restart Pingora\n5) View console log\n6) Purge Pingora\n7) Exit\nInput choice: ");
  }
  char opt;
  scanf(" %c", &opt);  // read chapter 5

  //presets
  if (opt == '1')
  {
    printf("");
  }

  if (opt == '2')
  {
    //
  }

  //status
  if (opt == '3')
  {
    printf("Press ctrl + C to exit\n");

  }

  return;
}