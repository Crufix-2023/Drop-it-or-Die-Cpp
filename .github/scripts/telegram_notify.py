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

def is_merge_commit(commit_message):
    """Проверяет, является ли коммит merge-коммитом"""
    return commit_message.startswith("Merge branch") or commit_message.startswith("Merge pull request")

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
    reply_markup = None

    if event_name == 'push':
        ref = event_data.get('ref', '')
        branch_name = ref.replace('refs/heads/', '')
        commits = event_data.get('commits', [])
        
        # Фильтруем коммиты (исключаем merge-коммиты)
        regular_commits = [commit for commit in commits if not is_merge_commit(commit.get('message', ''))]
        merge_commits = [commit for commit in commits if is_merge_commit(commit.get('message', ''))]
        
        # Отправляем обычные коммиты
        if regular_commits:
            branch_name_escaped = html.escape(branch_name)
            commits_text = ""
            
            for i, commit in enumerate(regular_commits[-10:]):
                commit_id = commit.get('id', '')[:7]
                commit_message = format_commit_message(commit.get('message', ''))
                commit_url = commit.get('url', '')
                commit_message_escaped = html.escape(commit_message)
                commit_author = commit.get('author', {}).get('name', sender_name_escaped)
                
                commits_text += f"• <a href=\"{commit_url}\">{commit_id}</a> - {commit_message_escaped} by {commit_author}\n"
            
            total_regular_commits = len(regular_commits)
            commit_text = "commit" if total_regular_commits == 1 else "commits"
            
            compare_url = f"{repo_url}/compare/{regular_commits[0]['id']}...{regular_commits[-1]['id']}"
            reply_markup = {
                "inline_keyboard": [[
                    {
                        "text": "Open Changes",
                        "url": compare_url
                    }
                ]]
            }
            
            message = (
                f'🔨 <b>{total_regular_commits} New {commit_text} to</b> <a href="{repo_url}">{repo_name_escaped}</a>[{branch_name_escaped}]\n\n'
                f'{commits_text}'
            )
            
        # Отправляем merge-коммиты отдельно
        for merge_commit in merge_commits:
            commit_id = merge_commit.get('id', '')[:7]
            commit_message = format_commit_message(merge_commit.get('message', ''))
            commit_url = merge_commit.get('url', '')
            commit_message_escaped = html.escape(commit_message)
            commit_author = merge_commit.get('author', {}).get('name', sender_name_escaped)
            
            merge_message = (
                f'🔄 <b>Merge commit to</b> <a href="{repo_url}">{repo_name_escaped}</a>[{html.escape(branch_name)}]\n\n'
                f'• <a href="{commit_url}">{commit_id}</a> - {commit_message_escaped} by {commit_author}'
            )
            
            print(f"Sending merge commit message: {merge_message}")
            result = send_telegram_message(merge_message)
            print(f"Telegram API response: {result}")
    
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
        result = send_telegram_message(message, reply_markup=reply_markup)
        print(f"Telegram API response: {result}")
    else:
        print(f"No message generated for event: {event_name}")

if __name__ == '__main__':
    main()









