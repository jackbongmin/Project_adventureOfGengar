#pragma once
// 범용 상태 머신. Owner(보스, PlayMode 등)가 현재 상태 객체를 하나 들고 있고,
// Change()는 현재 Update가 끝난 뒤에 적용되므로 상태 안에서 안전하게 호출할 수 있다.
#include <memory>

namespace gengar {

class Graphics;

template <class Owner>
class State {
 public:
  virtual ~State() = default;
  virtual void Enter(Owner&) {}
  virtual void Update(Owner&) = 0;
  virtual void Render(Owner&, Graphics&) {}
  virtual void Exit(Owner&) {}
  virtual const char* Name() const = 0;
};

template <class Owner>
class StateMachine {
 public:
  explicit StateMachine(Owner& owner) : owner_(owner) {}

  void Change(std::unique_ptr<State<Owner>> next) { pending_ = std::move(next); }

  void Update() {
    if (current_) current_->Update(owner_);
    Apply();
  }
  void Render(Graphics& g) {
    if (current_) current_->Render(owner_, g);
  }
  // 대기 중인 전환을 즉시 적용 (생성 직후 첫 상태 진입 등에 사용)
  void Apply() {
    while (pending_) {
      std::unique_ptr<State<Owner>> next = std::move(pending_);
      if (current_) current_->Exit(owner_);
      current_ = std::move(next);
      current_->Enter(owner_);
    }
  }
  State<Owner>* Current() const { return current_.get(); }
  const char* CurrentName() const { return current_ ? current_->Name() : "none"; }

 private:
  Owner& owner_;
  std::unique_ptr<State<Owner>> current_;
  std::unique_ptr<State<Owner>> pending_;
};

}  // namespace gengar
