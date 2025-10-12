#!/usr/bin/env python3
import os
import json
import requests
import html

def get_user_name(login):
    url = f"https://api.github.com/users/{login}"
    response = requests.get(url)
    
    if response.status_code == 200:
        user_data = response.json()
        return user_data.get('name', login)
    else:
        return login

def send_telegram_message(message, parse_mode='HTML', reply_markup=None):
    bot_token = os.getenv('TELEGRAM_BOT_TOKEN')
    chat_id = os.getenv('TELEGRAM_CHAT_ID')
    topic_id = 6
    
    url = f"https://api.telegram.org/bot{bot_token}/sendMessage"
    payload = {
        'chat_id': chat_id,
        'message_thread_id': topic_id,
        'text': message,
        'parse_mode': parse_mode,
        'disable_web_page_preview': True
    }

    if reply_markup:
        payload['reply_markup'] = reply_markup
    
    response = requests.post(url, json=payload)
    return response.json()

def format_commit_message(commit_message, max_length=50):
    """Форматирует сообщение коммита, обрезает если слишком длинное"""
    message = commit_message.split('\n')[0]  # Берем первую строку
    if len(message) > max_length:
        return message[:max_length] + '...'
    return message

def main():
    event_path = os.getenv('GITHUB_EVENT_PATH')
    
    with open(event_path, 'r') as f:
        event_data = json.load(f)
    
    event_name = os.getenv('GITHUB_EVENT_NAME')
    repo_name = event_data['repository']['full_name']
    repo_url = event_data['repository']['html_url']
    sender_login = event_data['sender']['login']
    sender_name = get_user_name(sender_login)
    
    # Экранируем HTML символы
    repo_name_escaped = html.escape(repo_name)
    sender_name_escaped = html.escape(sender_name)
    
    message = None

    if event_name == 'push':
        # Данные для push события
        ref = event_data.get('ref', '')
        branch_name = ref.replace('refs/heads/', '')
        commits = event_data.get('commits', [])
        total_commits = len(commits)
        
        if commits:
            branch_name_escaped = html.escape(branch_name)
            
            # Формируем список коммитов
            commits_text = ""
            for i, commit in enumerate(commits[-10:]):  # Показываем последние 10 коммитов
                commit_id = commit.get('id', '')[:7]
                commit_message = format_commit_message(commit.get('message', ''))
                commit_url = commit.get('url', '')
                commit_message_escaped = html.escape(commit_message)
                
                commits_text += f"• <a href=\"{commit_url}\">{commit_id}</a> - {commit_message_escaped} by {sender_name_escaped}\n"
            
            # Если коммитов больше 10, показываем количество скрытых
            commit_text = "commit"
            if total_commits > 1:
                commit_text = "commits"

            # Создаем кнопку со ссылкой на сравнение коммитов
            compare_url = f"{repo_url}/compare/{commits[0]['id']}...{commits[-1]['id']}"
            reply_markup = {
                "inline_keyboard": [[
                    {
                        "text": "Open Changes",
                        "url": compare_url
                    }
                ]]
            }
            
            message = (
                f'📥 <b>🔨 {total_commits} New {commit_text} to</b> <a href="{repo_url}">{repo_name_escaped}</a>[{branch_name_escaped}]\n\n'
                f'{commits_text}'
            )
    
    elif event_name == 'create':
        # Данные для create события (ветки и теги)
        event_type = event_data.get('ref_type', '')
        ref_name = event_data.get('ref', '')
        ref_name_escaped = html.escape(ref_name)
        
        if event_type == 'branch':
            message = (
                f'🔨 <b>New branch created in</b> <a href="{repo_url}">{repo_name_escaped}</a>\n'
                f'• <a href="{repo_url}/tree/{ref_name}">{ref_name_escaped}</a> by {sender_name_escaped}'
            )
        
        elif event_type == 'tag':
            message = (
                f'🏷️ <b>New tag created in</b> <a href="{repo_url}">{repo_name_escaped}</a>\n'
                f'• <a href="{repo_url}/releases/tag/{ref_name}">{ref_name_escaped}</a> by {sender_name_escaped}'
            )
    
    if message:
        print(f"Sending message: {message}")
        result = send_telegram_message(message)
        print(f"Telegram API response: {result}")
    else:
        print(f"No message generated for event: {event_name}")

if __name__ == '__main__':
    main()




